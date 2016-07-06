#include <unity/storage/qt/client/internal/remote_client/DeleteHandler.h>

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

DeleteHandler::DeleteHandler(QDBusPendingReply<void> const& reply, shared_ptr<ItemImpl> const& impl)
    : impl_(impl)
    , watcher_(reply, this)
{
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &DeleteHandler::finished);
    qf_.reportStarted();
}

QFuture<void> DeleteHandler::future()
{
    return qf_.future();
}

void DeleteHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<void> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO: remove this
        qf_.reportException(ResourceException("error"));  // TODO
    }
    impl_->deleted_ = true;
    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "DeleteHandler.moc"
