#include <unity/storage/qt/client/internal/remote_client/DownloaderImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Downloader.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/make_future.h>
#include <unity/storage/qt/client/internal/remote_client/FinishDownloadHandler.h>

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
namespace remote_client
{

DownloaderImpl::DownloaderImpl(QString const& download_id,
                               QDBusUnixFileDescriptor fd,
                               shared_ptr<File> const& file,
                               ProviderInterface& provider)
    : DownloaderBase(file)
    , download_id_(download_id)
    , fd_(fd)
    , file_(file)
    , provider_(provider)
    , read_socket_(new QLocalSocket)
{
    assert(!download_id.isEmpty());
    assert(fd.isValid());
    read_socket_->setSocketDescriptor(fd.fileDescriptor(), QLocalSocket::ConnectedState, QIODevice::ReadOnly);
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
    return read_socket_;
}

QFuture<void> DownloaderImpl::finish_download()
{
    auto handler = new FinishDownloadHandler(provider_.FinishDownload(download_id_));
    return handler->future();
}

QFuture<void> DownloaderImpl::cancel() noexcept
{
    read_socket_->abort();
    return make_ready_future();
}

Downloader::SPtr DownloaderImpl::make_downloader(QString const& download_id,
                                                 QDBusUnixFileDescriptor fd,
                                                 shared_ptr<File> const& file,
                                                 ProviderInterface& provider)
{
    auto impl = new DownloaderImpl(download_id, fd, file, provider);
    Downloader::SPtr downloader(new Downloader(impl));
    return downloader;
}


}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "DownloaderImpl.moc"
