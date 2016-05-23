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

DownloaderImpl::DownloaderImpl(weak_ptr<File> file)
{
    auto f = file.lock();
    assert(f);
    file_ = static_pointer_cast<File>(f);
    assert(file_);
}

DownloaderImpl::~DownloaderImpl()
{
    cancel();
}

shared_ptr<File> DownloaderImpl::file() const
{
    return file_;
}

shared_ptr<QLocalSocket> DownloaderImpl::socket() const
{
    switch (state_)
    {
        case uninitialized:
        {
            // TODO: What if upload/download already in progress?
            // TODO: What if file destroyed?
            int fds[2];
            int rc = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
            if (rc == -1)
            {
                throw StorageException();  // TODO
            }
            auto This = const_cast<DownloaderImpl*>(this);
            This->read_socket_ = make_shared<QLocalSocket>();
            This->read_socket_->setSocketDescriptor(fds[0], QLocalSocket::ConnectedState, QIODevice::ReadOnly);
            This->write_socket_ = make_shared<QLocalSocket>();
            This->write_socket_->setSocketDescriptor(fds[1], QLocalSocket::ConnectedState, QIODevice::WriteOnly);
            return This->read_socket_;
        }
        case connected:
        {
            return read_socket_;
        }
        case finalized:
        {
            throw StorageException();  // TODO
        }
        default:
        {
            abort();  // Impossible
        }
    }
    // NOTREACHED
}

QFuture<void> DownloaderImpl::close()
{
    if (state_ == connected)
    {
        write_socket_->disconnectFromServer();
        read_socket_->close();
        state_ = finalized;
    }
    QFutureInterface<void> qf;
    qf.reportFinished();
    return qf.future();
}

QFuture<void> DownloaderImpl::cancel()
{
    if (state_ == connected)
    {
        write_socket_->abort();
        read_socket_->abort();
        state_ = finalized;
    }
    QFutureInterface<void> qf;
    qf.reportFinished();
    return qf.future();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
