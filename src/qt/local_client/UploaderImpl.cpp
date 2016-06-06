#include <unity/storage/qt/client/internal/UploaderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/ItemImpl.h>
#include <unity/storage/qt/client/StorageSocket.h>

#include <boost/filesystem.hpp>

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

UploadWorker::UploadWorker(int read_fd,
                           shared_ptr<File> const& file,
                           ConflictPolicy policy,
                           QFutureInterface<TransferState>& qf)
    : read_fd_(read_fd)
    , file_(file)
    , tmp_fd_([](int fd){ if (fd != -1) ::close(fd); })
    , policy_(policy)
    , qf_(qf)
{
    assert(read_fd > 0);
}

void UploadWorker::start_uploading() noexcept
{
    read_socket_.reset(new StorageSocket);
    read_socket_->setSocketDescriptor(read_fd_, QLocalSocket::ConnectedState, QIODevice::ReadOnly);

    // Monitor read socket for ready-to-read, disconnected, and error events.
    connect(read_socket_.get(), &QLocalSocket::readyRead, this, &UploadWorker::on_bytes_ready);
    connect(read_socket_.get(), &QLocalSocket::disconnected, this, &UploadWorker::on_disconnected);
    connect(read_socket_.get(), static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
            this, &UploadWorker::on_error);

    using namespace boost::filesystem;

    // Open tmp file for writing.
    auto parent_path = path(file_->native_identity().toStdString()).parent_path();
    tmp_fd_.reset(open(parent_path.native().c_str(), O_TMPFILE | O_WRONLY, 0600));
    // TODO: O_TMPFILE may not work with some kernels. Fall back to conventional
    //       tmp file creation here in that case.
    assert(tmp_fd_.get() != -1);
    output_file_.reset(new QFile);
    output_file_->open(tmp_fd_.get(), QIODevice::WriteOnly, QFileDevice::DontCloseHandle);

    qf_.reportStarted();
}

// Called once we know the outcome of the upload, or via a signal when the client
// calls finish_download(). This makes the future ready with the appropriate
// result or error information. If the client has not disconnected
// yet, we don't touch the future; it becomes ready once we receive the disconnected signal.

void UploadWorker::do_finish()
{
    if (qf_.future().isFinished())
    {
        return;  // Future was set previously. no point in continuing.
    }

    switch (state_)
    {
        case in_progress:
        {
            if (disconnected_)
            {
                // Future doesn't become ready until the client has
                // disconnected from its write socket.
                state_ = finalized;
                finalize();
                qf_.reportResult(TransferState::ok);
                qf_.reportFinished();
            }
            break;
        }
        case finalized:
        {
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
    if (qf_.future().isFinished())
    {
        QThread::currentThread()->quit();
    }
}

// Called via signal from the client to stop things.

void UploadWorker::do_cancel()
{
    if (state_ == in_progress)
    {
        disconnect(read_socket_.get(), nullptr, this, nullptr);
        read_socket_->abort();
        state_ = cancelled;
        do_finish();
    }
}

void UploadWorker::on_bytes_ready()
{
    auto buf = read_socket_->read(read_socket_->bytesAvailable());
    auto bytes_written = output_file_->write(buf);
    if (bytes_written != buf.size())
    {
        // TODO: Store error details.
        handle_error();
        return;
    }
}

void UploadWorker::on_disconnected()
{
    disconnected_ = true;
    do_finish();
}

void UploadWorker::on_error()
{
    handle_error();
}

void UploadWorker::finalize()
{
    try
    {
        check_modified_time();  // Throws if time stamps don't match and policy is error_if_conflict.
    }
    catch (std::exception const&)
    {
        qf_.reportException(ConflictException());  // TODO: store error details.
        qf_.reportFinished();
        return;
    }

    if (!output_file_->flush())
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
    output_file_->close();
    file_->p_->update_modified_time();
    qf_.reportResult(TransferState::ok);
    qf_.reportFinished();
}

void UploadWorker::handle_error()
{
    if (state_ == in_progress)
    {
        output_file_->close();
        read_socket_->abort();
    }
    state_ = error;
    do_finish();
}

void UploadWorker::check_modified_time() const
{
    if (policy_ == ConflictPolicy::error_if_conflict)
    {
        // TODO: What if file does not yet exist on disk?
        auto mtime = boost::filesystem::last_write_time(file_->native_identity().toStdString());
        QDateTime modified_time;
        modified_time.setTime_t(mtime);
        if (modified_time != file_->last_modified_time())
        {
            throw ConflictException();  // TODO
        }
    }
}

UploadThread::UploadThread(UploadWorker* worker)
    : worker_(worker)
{
}

void UploadThread::run()
{
    worker_->start_uploading();
    exec();
}

UploaderImpl::UploaderImpl(weak_ptr<File> file, ConflictPolicy policy)
    : QObject(nullptr)
    , file_(file.lock())
    , policy_(policy)
{
    assert(file_);

    // Set up socket pair.
    int fds[2];
    int rc = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
    if (rc == -1)
    {
        throw StorageException();  // LCOV_EXCL_LINE  // TODO
    }

    // Write socket is for the client.
    write_socket_.reset(new StorageSocket);
    write_socket_->setSocketDescriptor(fds[1], QLocalSocket::ConnectedState, QIODevice::WriteOnly);

    // Create worker and connect slots, so we can sign the worker when the client calls
    // finish_download() or cancel();
    worker_.reset(new UploadWorker(fds[0], file_, policy, qf_));
    connect(this, &UploaderImpl::do_finish, worker_.get(), &UploadWorker::do_finish);
    connect(this, &UploaderImpl::do_cancel, worker_.get(), &UploadWorker::do_cancel);

    // Create upload thread and make sure that worker slots are called from the upload thread.
    upload_thread_.reset(new UploadThread(worker_.get()));
    worker_->moveToThread(upload_thread_.get());

    upload_thread_->start();

    // TODO: can probably do this with a signal?
    // There is no waitForStarted() on QFutureInterface or QFuture.
    while (!qf_.isStarted())
        ;
}

UploaderImpl::~UploaderImpl()
{
    Q_EMIT do_cancel();
    upload_thread_->wait();
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
    if (!qf_.future().isFinished())
    {
        // Set state of the future if that hasn't happened yet.
        Q_EMIT do_finish();
    }
    return qf_.future();
}

QFuture<void> UploaderImpl::cancel() noexcept
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
