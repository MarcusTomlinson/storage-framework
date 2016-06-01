#include <unity/storage/qt/client/internal/DownloaderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>

#include <QSocketNotifier>
#include <QtConcurrent>

#include <cassert>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

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
    , state_(connected)
    , file_(file.lock())
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
    if (!read_socket_->setSocketDescriptor(fds[0], QLocalSocket::ConnectedState, QIODevice::ReadOnly))
    {
        // LCOV_EXCL_START
        close(fds[0]);
        close(fds[1]);
        throw StorageException();  // TODO
        // LCOV_EXCL_STOP
    }
    write_socket_.reset(new StorageSocket);
    if (!write_socket_->setSocketDescriptor(fds[1], QLocalSocket::ConnectedState, QIODevice::WriteOnly))
    {
        // LCOV_EXCL_START
        close(fds[1]);
        throw StorageException();  // TODO
        // LCOV_EXCL_STOP
    }

    // Monitor write socket for ready-to-write and error events.
    write_notifier_.reset(new QSocketNotifier(write_socket_->socketDescriptor(), QSocketNotifier::Write));
    connect(write_notifier_.get(), &QSocketNotifier::activated, this, &DownloaderImpl::on_ready);
    error_notifier_.reset(new QSocketNotifier(write_socket_->socketDescriptor(), QSocketNotifier::Exception));
    connect(error_notifier_.get(), &QSocketNotifier::activated, this, &DownloaderImpl::on_error);

    // Monitor write socket for disconnection, so we can wait until all pending data was written.
    connect(write_socket_.get(), &QLocalSocket::disconnected, this, &DownloaderImpl::on_disconnect);

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
    switch (state_)
    {
        case connected:
        {
            // Do nothing, on_disconnect() makes the future ready.
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
    }
}

void DownloaderImpl::on_error()
{
    handle_error();
}

void DownloaderImpl::on_disconnect()
{
    qf_.reportResult(TransferState::ok);
    qf_.reportFinished();
    disconnect(write_socket_.get(), &QLocalSocket::disconnected, this, &DownloaderImpl::on_disconnect);
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
