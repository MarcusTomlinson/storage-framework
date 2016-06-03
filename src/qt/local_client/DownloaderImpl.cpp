#include <unity/storage/qt/client/internal/DownloaderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/StorageSocket.h>

#include <QLocalSocket>

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
    read_socket_->setSocketDescriptor(fds[0], QLocalSocket::ConnectedState, QIODevice::ReadOnly);
    write_socket_.reset(new StorageSocket);
    write_socket_->setSocketDescriptor(fds[1], QLocalSocket::ConnectedState, QIODevice::WriteOnly);

    // Monitor write socket for ready-to-write, disconnected, and error events.
    connect(write_socket_.get(), &QLocalSocket::bytesWritten, this, &DownloaderImpl::on_bytes_written);
    connect(write_socket_.get(), &QLocalSocket::disconnected, this, &DownloaderImpl::on_disconnected);
    connect(write_socket_.get(), static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
            this, &DownloaderImpl::on_error);

    // Open file for reading.
    input_file_.reset(new QFile(file_->native_identity()));
    if (!input_file_->open(QIODevice::ReadOnly))
    {
        throw StorageException();  // TODO
    }
    bytes_to_write_ = input_file_->size();
    if (bytes_to_write_ == 0)
    {
        write_socket_->disconnectFromServer();  // So the client gets EOF for empty files.
    }
    else
    {
        Q_EMIT on_ready();  // Kick off the read-write cycle.
    }
}

DownloaderImpl::~DownloaderImpl()
{
    cancel();  // noexcept
}

shared_ptr<File> DownloaderImpl::file() const
{
    return file_;
}

shared_ptr<QLocalSocket> DownloaderImpl::socket() const
{
    return read_socket_;
}

QFuture<TransferState> DownloaderImpl::finish_download()
{
    switch (state_)
    {
        case in_progress:
        {
            if (bytes_to_write_ > 0)
            {
                // Still unread data left, caller abandoned download early without cancelling.
                qf_.reportException(StorageException());  // TODO: logic error
                qf_.reportFinished();
            }
            // Else do nothing; on_disconnected() will make the future ready.
            break;
        }
        case finalized:
        {
            // finish_download() called more than once.
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
    if (state_ == in_progress)
    {
        write_socket_->abort();
        state_ = cancelled;
    }
    QFutureInterface<void> qf;
    qf.reportFinished();
    return qf.future();
}

void DownloaderImpl::on_ready()
{
    qint64 const READ_SIZE = 64 * 1024;

    QByteArray buf;
    buf.resize(READ_SIZE);
    auto bytes_read = input_file_->read(buf.data(), buf.size());
    if (bytes_read == -1)
    {
        // TODO: Store error details.
        handle_error();
        return;
    }
    buf.resize(bytes_read);

    auto bytes_written = write_socket_->write(buf);
    if (bytes_written == -1)
    {
        // TODO: Store error details.
        handle_error();
        return;
    }
}

void DownloaderImpl::on_bytes_written(qint64 bytes)
{
    bytes_to_write_ -= bytes;
    assert(bytes_to_write_ >= 0);
    if (bytes_to_write_ > 0)
    {
        Q_EMIT on_ready();
    }
    else
    {
        write_socket_->disconnectFromServer();
    }
}

void DownloaderImpl::on_disconnected()
{
    qf_.reportResult(TransferState::ok);
    qf_.reportFinished();
}

void DownloaderImpl::on_error()
{
    handle_error();
}

void DownloaderImpl::handle_error()
{
    if (state_ == in_progress)
    {
        input_file_->close();
        write_socket_->abort();
    }
    state_ = error;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "DownloaderImpl.moc"
