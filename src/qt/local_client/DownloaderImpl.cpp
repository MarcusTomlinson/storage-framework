#include <unity/storage/qt/client/internal/DownloaderImpl.h>

#include <unity/storage/common.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/StorageSocket.h>

#include <cassert>

#include <fcntl.h>
#include <sys/socket.h>

using namespace unity::storage::qt::client;
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

DownloaderImpl::DownloaderImpl(weak_ptr<File> file)
    : QObject(nullptr)
    , file_(file.lock())
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
        throw StorageException();
    }
    write_socket_.reset(new StorageSocket);
    if (!write_socket_->setSocketDescriptor(fds[1], QLocalSocket::ConnectedState, QIODevice::WriteOnly))
    {
        throw StorageException();
    }

    // Monitor write socket for write and error events.
    write_notifier_.reset(new QSocketNotifier(write_socket_->socketDescriptor(), QSocketNotifier::Write));
    connect(write_notifier_.get(), &QSocketNotifier::activated, this, &DownloaderImpl::on_ready);
    error_notifier_.reset(new QSocketNotifier(write_socket_->socketDescriptor(), QSocketNotifier::Exception));
    connect(error_notifier_.get(), &QSocketNotifier::activated, this, &DownloaderImpl::on_error);

    // Open file for reading.
    int fd = open(file_->native_identity().toStdString().c_str(), O_RDONLY);
    if (fd == -1)
    {
        throw StorageException();  // TODO
    }
    input_file_.open(fd, QIODevice::ReadOnly, QFileDevice::AutoCloseHandle);
}

DownloaderImpl::~DownloaderImpl()
{
    cancel();  // noexcept
}

shared_ptr<File> DownloaderImpl::file() const
{
    return file_;
}

shared_ptr<StorageSocket> DownloaderImpl::socket() const
{
    return read_socket_;
}

QFuture<TransferState> DownloaderImpl::finish_download()
{
    QFutureInterface<TransferState> qf;
    switch (state_)
    {
        case connected:
        {
            qf.reportException(StorageException());  // TODO, logic error
            break;
        }
        case completed:
        {
            state_ = finalized;
            qf.reportResult(TransferState::ok);
            break;
        }
        case finalized:
        {
            qf.reportException(StorageException());  // TODO, logic error
            break;
        }
        case error:
        {
            qf.reportException(StorageException());  // TODO, report details
            break;
        }
        case cancelled:
        {
            qf.reportResult(TransferState::cancelled);
            break;
        }
        default:
        {
            abort();  // Impossible
        }
    }
    qf.reportFinished();
    return qf.future();
}

QFuture<void> DownloaderImpl::cancel() noexcept
{
    if (state_ == connected)
    {
        write_notifier_->setEnabled(false);
        error_notifier_->setEnabled(false);
        write_socket_->abort();
        state_ = cancelled;
    }
    QFutureInterface<void> qf;
    qf.reportFinished();
    return qf.future();
}

void DownloaderImpl::on_ready()
{
    assert(state_ == connected);
    assert(pos_ >= 0);
    assert(pos_ <= end_);
    assert(end_ >= 0);

    if (!eof_ && pos_ == end_)
    {
        // Buffer empty, need to read a chunk.
        end_ = input_file_.read(buf_, StorageSocket::CHUNK_SIZE);
        if (end_ == -1)
        {
            // TODO: Store error details.
            handle_error();
            return;
        }
        eof_ = end_ < StorageSocket::CHUNK_SIZE;
        pos_ = 0;
    }

    int bytes_to_write = end_ - pos_;
    if (bytes_to_write > 0)
    {
        int bytes_written = write_socket_->writeData(buf_, bytes_to_write);
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
        write_notifier_->setEnabled(false);
        error_notifier_->setEnabled(false);
        write_socket_->disconnectFromServer();
        state_ = completed;
    }
}

void DownloaderImpl::on_error()
{
    handle_error();
}

void DownloaderImpl::handle_error()
{
    if (state_ == connected)
    {
        input_file_.close();
        write_notifier_->setEnabled(false);
        error_notifier_->setEnabled(false);
        write_socket_->abort();
        end_ = 0;
        pos_ = 0;
    }
    state_ = error;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "DownloaderImpl.moc"
