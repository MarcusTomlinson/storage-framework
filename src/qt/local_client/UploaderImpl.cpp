#include <unity/storage/qt/client/internal/UploaderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/StorageSocket.h>

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
    , state_(in_progress)
    , file_(file.lock())
    , policy_(policy)
    , tmp_fd_([](int fd){ if (fd != -1) ::close(fd); })
{
    assert(file_);

    // Set up socket pair.
    int fds[2];
    int rc = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
    if (rc == -1)
    {
        throw StorageException();  // LCOV_EXCL_LINE  // TODO
    }
    read_socket_.reset(new StorageSocket);
    read_socket_->setSocketDescriptor(fds[0], QLocalSocket::ConnectedState, QIODevice::ReadOnly);
    write_socket_.reset(new StorageSocket);
    write_socket_->setSocketDescriptor(fds[1], QLocalSocket::ConnectedState, QIODevice::WriteOnly);

    // Monitor read socket for ready-to-read, disconnected, and error events.
    connect(read_socket_.get(), &QLocalSocket::readyRead, this, &UploaderImpl::on_ready);
    connect(read_socket_.get(), &QLocalSocket::disconnected, this, &UploaderImpl::on_disconnected);
    connect(read_socket_.get(), static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
            this, &UploaderImpl::on_error);

    using namespace boost::filesystem;

    // Open tmp file for writing.
    auto parent_path = path(file_->native_identity().toStdString()).parent_path();
    tmp_fd_.reset(open(parent_path.native().c_str(), O_TMPFILE | O_WRONLY, 0600));
    if (tmp_fd_.get() == -1)
    {
        // TODO: O_TMPFILE may not work with some kernels. Fall back to conventional
        //       tmp file creation here in that case.
        throw StorageException();  // TODO
    }
    output_file_.open(tmp_fd_.get(), QIODevice::WriteOnly, QFileDevice::DontCloseHandle);

    check_modified_time();  // Throws if file has been changed on disk and policy is error_if_conflict.
}

UploaderImpl::~UploaderImpl()
{
    read_socket_.reset();  // Disconnect everything
    if (!received_something_ && state_ == in_progress)
    {
        // Caller never wrote anything and just let the uploader go
        // out of scope without calling finish_upload().
        // In that case, we finalize so we leave an empty file behind.
        finalize();
        return;
    }
    cancel();  // noexcept
}

shared_ptr<File> UploaderImpl::file() const
{
    return file_;
}

shared_ptr<QLocalSocket> UploaderImpl::socket() const
{
    return write_socket_;
}

QFuture<TransferState> UploaderImpl::finish_upload()
{
    switch (state_)
    {
        case in_progress:
        {
            if (!disconnected_ && received_something_)
            {
                // Caller created an uploader and wrote something, but did not
                // disconnect the socket, so the transfer is incomplete.
                qf_.reportException(StorageException());  // TODO, logic error
                qf_.reportFinished();
            }
            // Else do nothing; on_disconnected() will make the future ready.
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
    if (state_ == in_progress)
    {
        state_ = cancelled;
        read_socket_->abort();
    }
    finish_upload();

    QFutureInterface<void> qf;
    qf.reportFinished();
    return qf.future();
}

void UploaderImpl::on_ready()
{
    received_something_ = true;

    auto buf = read_socket_->read(read_socket_->bytesAvailable());
    auto bytes_written = output_file_.write(buf);
    if (bytes_written == -1)
    {
        // TODO: Store error details.
        handle_error();
        return;
    }
}

void UploaderImpl::on_disconnected()
{
    disconnected_ = true;
    finalize();
}

void UploaderImpl::on_error()
{
    handle_error();
}

void UploaderImpl::finalize()
{
    try
    {
        check_modified_time();  // Throws if time stamps don't match and policy is error_if_conflict.
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
    string oldpath = string("/proc/self/fd/") + std::to_string(tmp_fd_.get());
    string newpath = file_->native_identity().toStdString();
    ::unlink(newpath.c_str());  // linkat() will not remove existing file: http://lwn.net/Articles/559969/
    if (linkat(-1, oldpath.c_str(), tmp_fd_.get(), newpath.c_str(), AT_SYMLINK_FOLLOW) == -1)
    {
        state_ = error;
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    state_ = finalized;
    qf_.reportResult(TransferState::ok);
    qf_.reportFinished();
}

void UploaderImpl::handle_error()
{
    if (state_ == in_progress)
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
