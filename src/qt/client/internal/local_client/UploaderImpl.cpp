#include <unity/storage/qt/client/internal/local_client/UploaderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/local_client/FileImpl.h>
#include <unity/storage/qt/client/internal/local_client/tmpfile-prefix.h>

#include <QLocalSocket>

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
namespace local_client
{

UploadWorker::UploadWorker(int read_fd,
                           weak_ptr<File> file,
                           int64_t size,
                           QString const& path,
                           ConflictPolicy policy,
                           weak_ptr<Root> root,
                           QFutureInterface<shared_ptr<File>>& qf,
                           QFutureInterface<void>& worker_initialized)
    : read_fd_(read_fd)
    , file_(file)
    , size_(size)
    , path_(path)
    , root_(root)
    , tmp_fd_([](int fd){ if (fd != -1) ::close(fd); })
    , policy_(policy)
    , qf_(qf)
    , worker_initialized_(worker_initialized)
{
    assert(read_fd > 0);
    assert(size >= 0);
    qf_.reportStarted();
    worker_initialized_.reportStarted();
}

void UploadWorker::start_uploading() noexcept
{
    read_socket_.reset(new QLocalSocket);
    read_socket_->setSocketDescriptor(read_fd_, QLocalSocket::ConnectedState, QIODevice::ReadOnly);
    shutdown(read_fd_, SHUT_WR);

    // Monitor read socket for ready-to-read, disconnected, and error events.
    connect(read_socket_.get(), &QLocalSocket::readyRead, this, &UploadWorker::on_bytes_ready);
    connect(read_socket_.get(), &QIODevice::readChannelFinished, this, &UploadWorker::on_read_channel_finished);
    connect(read_socket_.get(), static_cast<void(QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
            this, &UploadWorker::on_error);

    using namespace boost::filesystem;

    // Open tmp file for writing.
    auto parent_path = path(path_.toStdString()).parent_path();
    tmp_fd_.reset(open(parent_path.native().c_str(), O_TMPFILE | O_WRONLY, 0600));
    if (tmp_fd_.get() == -1)
    {
        // LCOV_EXCL_START
        // Some kernels on the phones don't support O_TMPFILE and return various errno values when this fails.
        // So, if anything at all goes wrong, we fall back on conventional temp file creation and
        // produce a hard error if that doesn't work either.
        string tmpfile = parent_path.native() + "/" + TMPFILE_PREFIX + "XXXXXX";
        tmp_fd_.reset(mkstemp(const_cast<char*>(tmpfile.data())));
        if (tmp_fd_.get() == -1)
        {
            worker_initialized_.reportFinished();
            state_ = error;
            // TODO: store error details.
            do_finish();
            return;
        }
        unlink(tmpfile.data());
        // LCOV_EXCL_STOP
    }
    output_file_.reset(new QFile);
    output_file_->open(tmp_fd_.get(), QIODevice::WriteOnly, QFileDevice::DontCloseHandle);

    worker_initialized_.reportFinished();
}

// Called once we know the outcome of the upload, or via a signal when the client
// calls finish_upload(). This makes the future ready with the appropriate
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
            state_ = finalized;
            finalize();
            break;
        }
        case finalized:
        {
            abort();  // LCOV_EXCL_LINE  // Impossible. If we get here, our logic is broken.
        }
        case cancelled:
        {
            qf_.reportException(CancelledException());  // TODO: details
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
    if (buf.size() != 0)
    {
        auto bytes_written = output_file_->write(buf);
        if (bytes_written != buf.size())
        {
            // TODO: Store error details.
            handle_error();
            return;
        }
    }
}

void UploadWorker::on_read_channel_finished()
{
    on_bytes_ready();  // In case there is still buffered data to be read.
    do_finish();
}

void UploadWorker::on_error()
{
    handle_error();
}

void UploadWorker::finalize()
{
    auto file = file_.lock();
    shared_ptr<FileImpl> impl;
    if (file)
    {
        // Upload is for a pre-existing file.
        impl = dynamic_pointer_cast<FileImpl>(file->p_);

        auto lock = impl->get_lock();

        try
        {
            check_modified_time();  // Throws if time stamps don't match and policy is error_if_conflict.
            impl->update_modified_time();
        }
        catch (std::exception const&)
        {
            qf_.reportException(ConflictException());  // TODO: store error details.
            qf_.reportFinished();
            return;
        }
    }
    else
    {
        // This uploader was returned by FolderImpl::create_file().
        int fd = open(path_.toStdString().c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);  // Fails if path already exists.
        if (fd == -1)
        {
            qf_.reportException(StorageException());  // TODO
        }
        if (close(fd) == -1)
        {
            qf_.reportException(StorageException());  // TODO
        }

        file = FileImpl::make_file(path_, root_);
        impl = dynamic_pointer_cast<FileImpl>(file->p_);
        auto lock = impl->get_lock();
        impl->update_modified_time();
    }

    if (!output_file_->flush())
    {
        qf_.reportException(StorageException());  // TODO: Store error details.
        qf_.reportFinished();
        return;
    }

    // Link the anonymous tmp file into the file system.
    string oldpath = string("/proc/self/fd/") + std::to_string(tmp_fd_.get());
    string newpath = file->native_identity().toStdString();
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
    qf_.reportResult(file);
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
        auto file = file_.lock();
        assert(file);
        auto mtime = boost::filesystem::last_write_time(file->native_identity().toStdString());
        QDateTime modified_time;
        modified_time.setTime_t(mtime);
        auto file_impl = dynamic_pointer_cast<FileImpl>(file->p_);
        if (modified_time != file_impl->get_modified_time())
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

UploaderImpl::UploaderImpl(weak_ptr<File> file,
                           int64_t size,
                           QString const& path,
                           ConflictPolicy policy,
                           weak_ptr<Root> root)
    : UploaderBase(policy, size)
{
    // Set up socket pair.
    int fds[2];
    int rc = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
    if (rc == -1)
    {
        throw StorageException();  // LCOV_EXCL_LINE  // TODO
    }

    // Write socket is for the client.
    write_socket_.reset(new QLocalSocket);
    write_socket_->setSocketDescriptor(fds[1], QLocalSocket::ConnectedState, QIODevice::WriteOnly);
    shutdown(fds[1], SHUT_RD);

    // Create worker and connect slots, so we can signal the worker when the client calls
    // finish_download() or cancel();
    QFutureInterface<void> worker_initialized;
    worker_.reset(new UploadWorker(fds[0], file, size, path, policy, root, qf_, worker_initialized));
    connect(this, &UploaderImpl::do_finish, worker_.get(), &UploadWorker::do_finish);
    connect(this, &UploaderImpl::do_cancel, worker_.get(), &UploadWorker::do_cancel);

    // Create upload thread and make sure that worker slots are called from the upload thread.
    upload_thread_.reset(new UploadThread(worker_.get()));
    worker_->moveToThread(upload_thread_.get());

    upload_thread_->start();

    worker_initialized.waitForFinished();
}

UploaderImpl::~UploaderImpl()
{
    Q_EMIT do_cancel();
    upload_thread_->wait();
}

shared_ptr<QLocalSocket> UploaderImpl::socket() const
{
    return write_socket_;
}

QFuture<File::SPtr> UploaderImpl::finish_upload()
{
    if (write_socket_->state() == QLocalSocket::ConnectedState)
    {
        write_socket_->disconnectFromServer();
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

}  // namespace local_client
}  // namespace intternal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
