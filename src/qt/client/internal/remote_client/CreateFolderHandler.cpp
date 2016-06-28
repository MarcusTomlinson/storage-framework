#include <unity/storage/qt/client/internal/remote_client/CreateFolderHandler.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/dbusmarshal.h>
#include <unity/storage/qt/client/internal/remote_client/FolderImpl.h>

#include <cassert>

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

CreateFolderHandler::CreateFolderHandler(QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                                         weak_ptr<Root> const& root)
    : watcher_(reply, this)
    , root_(root.lock())
{
    assert(root_);
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &CreateFolderHandler::finished);
    qf_.reportStarted();
}

QFuture<shared_ptr<Folder>> CreateFolderHandler::future()
{
    return qf_.future();
}

void CreateFolderHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<storage::internal::ItemMetadata> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    shared_ptr<Item> item;
    auto md = reply.value();
    if (md.type != ItemType::folder)
    {
        qf_.reportException(StorageException());  // TODO need to log this as well, server error
    }
    else
    {
        qf_.reportResult(FolderImpl::make_folder(md, root_));
    }
    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "CreateFolderHandler.moc"
