#include <unity/storage/qt/client/internal/DownloaderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>

#include <QLocalSocket>

#include <cassert>

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

DownloadWorker::DownloadWorker(int write_fd, QString const& filename, QFutureInterface<TransferState>& qf)
    : write_fd_(write_fd)
    , filename_(filename)
    , qf_(qf)
{
    assert(write_fd >= 0);
}

void DownloadWorker::start_downloading() noexcept
{
    write_socket_.reset(new QLocalSocket);
    write_socket_->setSocketDescriptor(write_fd_, QLocalSocket::ConnectedState, QIODevice::WriteOnly);

    // Monitor write socket for ready-to-write, disconnected, and error events.
    connect(write_socket_.get(), &QLocalSocket::bytesWritten, this, &DownloadWorker::on_bytes_written);
    connect(write_socket_.get(), &QLocalSocket::disconnected, this, &DownloadWorker::on_disconnected);
    connect(write_socket_.get(), static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
            this, &DownloadWorker::on_error);

    // Open file for reading.
    input_file_.reset(new QFile(filename_));
    if (!input_file_->open(QIODevice::ReadOnly))
    {
        // TODO, set details
        handle_error();
        return;
    }
    bytes_to_write_ = input_file_->size();

    qf_.reportStarted();

    if (bytes_to_write_ == 0)
    {
        write_socket_->disconnectFromServer();  // So the client gets EOF for empty files.
    }
    else
    {
        read_and_write_chunk();  // Kick off the read-write cycle.
    }
}

// Called once we know the outcome of the download, or via a signal when the client
// calls finish_download(). This makes the future ready with the appropriate
// result or error information.

void DownloadWorker::do_finish()
{
    switch (state_)
    {
        case in_progress:
        {
            if (bytes_to_write_ > 0)
            {
                // Still unwrittten data left, caller abandoned download early without cancelling.
                state_ = error;  // TODO, store details
                qf_.reportException(StorageException());  // TODO: logic error
                qf_.reportFinished();
            }
            else
            {
                state_ = finalized;
                qf_.reportResult(TransferState::ok);
                qf_.reportFinished();
            }
            break;
        }
        case finalized:
        {
            // finish_download() called more than once.
            qf_.reportException(StorageException());  // TODO, logic error
            qf_.reportFinished();
            break;
        }
        case cancelled:
        {
            qf_.reportResult(TransferState::cancelled);
            qf_.reportFinished();
            break;
        }
        case error:
        {
            qf_.reportException(StorageException());  // TODO, report details
            qf_.reportFinished();
            break;
        }
        default:
        {
            abort();  // LCOV_EXCL_LINE  // Impossible
        }
    }
    assert(qf_.future().isFinished());
    QThread::currentThread()->quit();
}

// Called via signal from the client to stop things.

void DownloadWorker::do_cancel()
{
    if (state_ == in_progress)
    {
        disconnect(write_socket_.get(), nullptr, this, nullptr);
        write_socket_->abort();
        bytes_to_write_ = 0;
        state_ = cancelled;
        do_finish();
    }
}

// Called each time we get rid of a chunk of data, to kick off the next chunk.

void DownloadWorker::on_bytes_written(qint64 bytes)
{
    bytes_to_write_ -= bytes;
    assert(bytes_to_write_ >= 0);
    if (bytes_to_write_ == 0)
    {
        input_file_->close();
        write_socket_->disconnectFromServer();
    }
    else
    {
        read_and_write_chunk();
    }
}

// Sets the outcome of the download in the future once we have written
// the last of the data and have disconnected.

void DownloadWorker::on_disconnected()
{
    do_finish();
}

void DownloadWorker::on_error()
{
    handle_error();
}

// Read the next chunk of data from the input file and write it to the socket.

void DownloadWorker::read_and_write_chunk()
{
    static qint64 constexpr READ_SIZE = 64 * 1024;

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
    if (bytes_written != bytes_read)
    {
        // TODO: Store error details.
        handle_error();
        return;
    }
}

void DownloadWorker::handle_error()
{
    if (state_ == in_progress)
    {
        write_socket_->abort();
    }
    state_ = error;
    do_finish();
}

DownloadThread::DownloadThread(DownloadWorker* worker)
    : worker_(worker)
{
}

void DownloadThread::run()
{
    worker_->start_downloading();
    exec();
}

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

    // Read socket is for the client.
    read_socket_.reset(new QLocalSocket);
    read_socket_->setSocketDescriptor(fds[0], QLocalSocket::ConnectedState, QIODevice::ReadOnly);

    // Create worker and connect slots, so we can signal the worker when the client calls
    // finish_download() or cancel().
    worker_.reset(new DownloadWorker(fds[1], file_->native_identity(), qf_));
    connect(this, &DownloaderImpl::do_finish, worker_.get(), &DownloadWorker::do_finish);
    connect(this, &DownloaderImpl::do_cancel, worker_.get(), &DownloadWorker::do_cancel);

    // Create download thread and make sure that worker slots are called from the download thread.
    download_thread_.reset(new DownloadThread(worker_.get()));
    worker_->moveToThread(download_thread_.get());

    download_thread_->start();

    // TODO: can probably do this with a signal?
    // This is no waitForStarted() on QFutureInterface or QFuture.
    while (!qf_.isStarted())
        ;
}

DownloaderImpl::~DownloaderImpl()
{
    Q_EMIT do_cancel();
    download_thread_->wait();
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
    if (!qf_.future().isFinished())
    {
        // Set state of the future if that hasn't happened yet.
        Q_EMIT do_finish();
    }
    return qf_.future();
}

QFuture<void> DownloaderImpl::cancel() noexcept
{
    Q_EMIT do_cancel();
    QFutureInterface<void> qf;
    qf.reportFinished();
    return qf.future();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
