#include <unity/storage/qt/client/internal/remote_client/CancelUploadHandler.h>

#include "ProviderInterface.h"
#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>

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

CancelUploadHandler::CancelUploadHandler(QDBusPendingReply<void> const& reply)
    : watcher_(reply, this)
{
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &CancelUploadHandler::finished);
    qf_.reportStarted();
}

QFuture<void> CancelUploadHandler::future()
{
    return qf_.future();
}

void CancelUploadHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<void> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
    }
    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "CancelUploadHandler.moc"
