#include <unity/storage/qt/client/internal/UploaderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>

#include <boost/filesystem.hpp>
#include <QSocketNotifier>
#include <QtConcurrent>

#include <cassert>

#include <fcntl.h>
#include <sys/socket.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{

UploaderImpl::UploaderImpl(weak_ptr<File> file, ConflictPolicy policy)
    : QObject(nullptr)
    , state_(connected)
    , file_(file.lock())
    , policy_(policy)
{
    assert(file_);

    // Set up socket pair.
    int fds[2];
    int rc = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
    if (rc == -1)
    {
        throw StorageException();  // TODO
    }
    read_socket_.reset(new StorageSocket);
    if (!read_socket_->setSocketDescriptor(fds[0], QLocalSocket::ConnectedState, QIODevice::ReadOnly))
    {
        close(fds[0]);
        close(fds[1]);
        throw StorageException();
    }
    write_socket_.reset(new StorageSocket);
    if (!write_socket_->setSocketDescriptor(fds[1], QLocalSocket::ConnectedState, QIODevice::WriteOnly))
    {
        close(fds[1]);
        throw StorageException();
    }

    // Monitor read socket for ready-to-read and error events.
    read_notifier_.reset(new QSocketNotifier(read_socket_->socketDescriptor(), QSocketNotifier::Read));
    connect(read_notifier_.get(), &QSocketNotifier::activated, this, &UploaderImpl::on_ready);
    error_notifier_.reset(new QSocketNotifier(read_socket_->socketDescriptor(), QSocketNotifier::Exception));
    connect(error_notifier_.get(), &QSocketNotifier::activated, this, &UploaderImpl::on_error);

    // Monitor write socket for disconnection. The disconnected signal can arrive while
    // data still remains to be read. We also monitor bytesWritten so, if the socket is
    // disconnected without ever having been written to, we can detect that we are at EOF.
    connect(write_socket_.get(), &QLocalSocket::disconnected, this, &UploaderImpl::on_disconnect);
    connect(write_socket_.get(), &QLocalSocket::bytesWritten, this, &UploaderImpl::on_write);

    using namespace boost::filesystem;

    // Open tmp file for writing.
    auto parent_path = path(file_->native_identity().toStdString()).parent_path();
    fd_ = open(parent_path.native().c_str(), O_TMPFILE | O_WRONLY, 0600);
    if (fd_ == -1)
    {
        // TODO: O_TMPFILE may not work with some kernels. Fall back to conventional
        //       tmp file creation here in that case.
        throw StorageException();  // TODO
    }
    output_file_.open(fd_, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle);

    check_modified_time();  // Throws if file has been changed on disk and policy is error_if_conflict.
}

UploaderImpl::~UploaderImpl()
{
    cancel();  // noexcept
}

shared_ptr<File> UploaderImpl::file() const
{
    return file_;
}

shared_ptr<StorageSocket> UploaderImpl::socket() const
{
    return write_socket_;
}

QFuture<TransferState> UploaderImpl::finish_upload()
{
    switch (state_)
    {
        case connected:
        {
            write_socket_->disconnectFromServer();
            state_ = disconnected;
            break;
        }
        case disconnected:
        {
            qf_.reportException(StorageException());  // TODO, logic error
            qf_.reportFinished();
            break;
        }
        case finalized:
        {
            qf_.reportException(StorageException());  // TODO, logic error
            qf_.reportFinished();
            break;
        }
        case error:
        {
            qf_.reportException(StorageException());  // TODO, report details
            qf_.reportFinished();
            break;
        }
        case cancelled:
        {
            qf_.reportResult(TransferState::cancelled);
            qf_.reportFinished();
            break;
        }
        default:
        {
            abort();  // Impossible
        }
    }
    return qf_.future();
}

QFuture<void> UploaderImpl::cancel() noexcept
{
    if (state_ == connected)
    {
        state_ = cancelled;
        write_socket_->abort();
    }
    QFutureInterface<void> qf;
    qf.reportFinished();
    return qf.future();
}

void UploaderImpl::on_ready()
{
    // on_ready() may not pop after disconnecting the write socket even though
    // more data remains to be read. So, each time we get on_ready, we read
    // all the data that is available.

    char buf[StorageSocket::CHUNK_SIZE];
    qint64 bytes_read;
    do
    {
        bytes_read = read_socket_->readData(buf, sizeof(buf));
        if (bytes_read == -1)
        {
            // TODO: Store error details.
            handle_error();
            return;
        }
        if (bytes_read != 0)
        {
            auto bytes_written = output_file_.write(buf, bytes_read);
            if (bytes_written < bytes_read)
            {
                // TODO: Store error details.
                handle_error();
                return;
            }
        }
    } while (bytes_read > 0);
    if (disconnected_)
    {
        finalize();
    }
}

void UploaderImpl::on_write(qint64 /* bytes_written */)
{
    // Record when the client writes to the socket, so we finalize
    // correctly if the client calls finish_upload() without
    // having written anything.
    bytes_written_ = true;
    // We don't need to handle this signal anymore.
    disconnect(write_socket_.get(), &QLocalSocket::bytesWritten, this, &UploaderImpl::on_write);
}

void UploaderImpl::on_error()
{
    handle_error();
}

void UploaderImpl::on_disconnect()
{
    disconnected_ = true;
    // If nothing was ever written, we need to call finalize() here
    // because on_ready() will not have been called.
    if (!bytes_written_)
    {
        finalize();
    }
}

void UploaderImpl::finalize()
{
    try
    {
        check_modified_time();  // Throws if time stamps don't match
    }
    catch (std::exception const&)
    {
        qf_.reportException(StorageException());  // TODO, version mismatch
        qf_.reportFinished();
        return;
    }

    if (!output_file_.flush())
    {
        qf_.reportException(StorageException());  // TODO: Store error details.
        qf_.reportFinished();
        return;
    }

    // Link the anonymous tmp file into the file system.
    string oldpath = string("/proc/self/fd/") + std::to_string(fd_);
    string newpath = file_->native_identity().toStdString();
    ::unlink(newpath.c_str());  // linkat() will not remove existing file: http://lwn.net/Articles/559969/
    if (linkat(AT_FDCWD, oldpath.c_str(), AT_FDCWD, newpath.c_str(), AT_SYMLINK_FOLLOW) == -1)
    {
        state_ = error;
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    output_file_.close();
    state_ = finalized;
    qf_.reportResult(TransferState::ok);
    qf_.reportFinished();
}

void UploaderImpl::handle_error()
{
    if (state_ == connected)
    {
        output_file_.close();  // Dubious
        read_socket_->abort();
    }
    state_ = error;
}

void UploaderImpl::check_modified_time() const
{

    if (policy_ == ConflictPolicy::error_if_conflict)
    {
        // TODO: What if file has been deleted on disk?
        auto mtime = boost::filesystem::last_write_time(file_->native_identity().toStdString());
        QDateTime modified_time;
        modified_time.setTime_t(mtime);
        if (modified_time != file_->last_modified_time())
        {
            throw StorageException();  // TODO
        }
    }
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "UploaderImpl.moc"
