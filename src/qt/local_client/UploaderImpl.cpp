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

    // Monitor read socket for read and error events.
    read_notifier_.reset(new QSocketNotifier(read_socket_->socketDescriptor(), QSocketNotifier::Read));
    connect(read_notifier_.get(), &QSocketNotifier::activated, this, &UploaderImpl::on_ready);
    error_notifier_.reset(new QSocketNotifier(read_socket_->socketDescriptor(), QSocketNotifier::Exception));
    connect(error_notifier_.get(), &QSocketNotifier::activated, this, &UploaderImpl::on_error);

    check_modified_time();  // Throws if file has been changed on disk and policy is error_if_conflict.

    using namespace boost::filesystem;

    // Open tmp file for writing.
    auto parent_path = path(file_->native_identity().toStdString()).parent_path();
    int fd_ = open(parent_path.native().c_str(), O_TMPFILE | O_WRONLY, 0600);
    if (fd_ == -1)
    {
        // TODO: O_TMPFILE may not work with some kernels. Fall back to conventional
        //       tmp file creation here in that case.
        throw StorageException();  // TODO
    }
    output_file_.open(fd_, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle);
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
    auto This = shared_from_this();  // Keep this uploader alive while the lambda is alive.
    auto finish = [This]()
    {
        switch (This->state_)
        {
            case connected:
            {
                This->write_socket_->disconnectFromServer();
                // TODO: Unfortunately, this emits a warning: https://bugreports.qt.io/browse/QTBUG-50711
                This->write_socket_->waitForDisconnected(-1);
                This->check_modified_time();  // Throws if time stamps don't match
                if (!This->output_file_.flush())
                {
                    This->state_ = error;
                    throw StorageException();  // TODO
                }
                string oldpath = string("/proc/self/fd/") + to_string(This->fd_);
                string newpath = This->file_->native_identity().toStdString();
                ::unlink(newpath.c_str());  // linkat() will not remove existing file: http://lwn.net/Articles/559969/
                if (linkat(-1, oldpath.c_str(), This->fd_, newpath.c_str(), 0) == -1)
                {
                    This->state_ = error;
                    throw StorageException();  // TODO
                }
                This->state_ = finalized;
                return TransferState::ok;
            }
            case finalized:
            {
                throw StorageException();  // TODO, logic error
            }
            case error:
            {
                throw StorageException();  // TODO, report details
            }
            case cancelled:
            {
                return TransferState::cancelled;
            }
            default:
            {
                abort();  // Impossible
            }
        }
    };
    return QtConcurrent::run(finish);
}

QFuture<void> UploaderImpl::cancel() noexcept
{
    if (state_ == connected)
    {
        read_notifier_->setEnabled(false);
        error_notifier_->setEnabled(false);
        read_socket_->abort();
        state_ = cancelled;
    }
    QFutureInterface<void> qf;
    qf.reportFinished();
    return qf.future();
}

void UploaderImpl::on_ready()
{
    assert(pos_ >= 0);
    assert(pos_ <= end_);
    assert(end_ >= 0);

    if (state_ != connected)
    {
        return;
    }

    if (!eof_ && pos_ == end_)
    {
        // Buffer empty, need to read a chunk.
        end_ = read_socket_->readData(buf_, StorageSocket::CHUNK_SIZE);
        if (end_ == -1)
        {
            // TODO: Store error details.
            handle_error();
            return;
        }
        eof_ = read_socket_->atEnd();
        pos_ = 0;
    }

    int bytes_to_write = end_ - pos_;
    if (bytes_to_write > 0)
    {
        int bytes_written = output_file_.write(buf_, bytes_to_write);
        if (bytes_written == -1)
        {
            // TODO: Store error details.
            handle_error();
            return;
        }
        pos_ += bytes_written;
    }
    if (eof_ && pos_ == end_)
    {
        read_notifier_->setEnabled(false);
        error_notifier_->setEnabled(false);
        read_socket_->disconnectFromServer();
    }
}

void UploaderImpl::on_error()
{
    handle_error();
}

void UploaderImpl::handle_error()
{
    if (state_ == connected)
    {
        output_file_.close();
        read_notifier_->setEnabled(false);
        error_notifier_->setEnabled(false);
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
