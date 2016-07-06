#include <unity/storage/qt/client/internal/remote_client/DownloadHandler.h>

#include <unity/storage/qt/client/Downloader.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/DownloaderImpl.h>

#include <QDBusUnixFileDescriptor>

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

DownloadHandler::DownloadHandler(QDBusPendingReply<QString, QDBusUnixFileDescriptor> const& reply,
                                 shared_ptr<File> const& file,
                                 ProviderInterface& provider)
    : file_(file)
    , provider_(provider)
    , watcher_(reply, this)
{
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &DownloadHandler::finished);
    qf_.reportStarted();
}

QFuture<shared_ptr<Downloader>> DownloadHandler::future()
{
    return qf_.future();
}

void DownloadHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<QString, QDBusUnixFileDescriptor> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    auto download_id = reply.argumentAt<0>();
    auto fd = reply.argumentAt<1>();
    if (fd.fileDescriptor() < 0)
    {
        qf_.reportException(StorageException());  // TODO
    }
    else
    {
        auto downloader = DownloaderImpl::make_downloader(download_id, fd, file_, provider_);
        qf_.reportResult(downloader);
    }

    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "DownloadHandler.moc"
