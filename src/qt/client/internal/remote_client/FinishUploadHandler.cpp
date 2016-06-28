#include <unity/storage/qt/client/internal/remote_client/FinishUploadHandler.h>

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

FinishUploadHandler::FinishUploadHandler(QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                                         weak_ptr<Root> root)
    : root_(root.lock())
    , watcher_(reply, this)
{
    assert(root_);
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &FinishUploadHandler::finished);
    qf_.reportStarted();
}

QFuture<shared_ptr<File>> FinishUploadHandler::future()
{
    return qf_.future();
}

void FinishUploadHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<storage::internal::ItemMetadata> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    auto md = reply.value();
    if (md.type != ItemType::file)
    {
        // Log this, server error
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }
    qf_.reportResult(FileImpl::make_file(md, root_));
    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "FinishUploadHandler.moc"
