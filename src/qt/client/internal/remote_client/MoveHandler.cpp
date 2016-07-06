#include <unity/storage/qt/client/internal/remote_client/MoveHandler.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/Folder.h>
#include <unity/storage/qt/client/internal/remote_client/dbusmarshal.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
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

MoveHandler::MoveHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                         weak_ptr<Root> const& root)
    : watcher_(reply, this)
    , root_(root.lock())
{
    assert(root_);
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &MoveHandler::finished);
    qf_.reportStarted();
}

QFuture<shared_ptr<Item>> MoveHandler::future()
{
    return qf_.future();
}

void MoveHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<storage::internal::ItemMetadata> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(ResourceException("error"));  // TODO
        qf_.reportFinished();
        return;
    }

    auto md = reply.value();
    shared_ptr<Item> item;
    switch (md.type)
    {
        case ItemType::file:
        {
            item = FileImpl::make_file(md, root_);
            break;
        }
        case ItemType::folder:
        {
            item = FolderImpl::make_folder(md, root_);
            break;
        }
        case ItemType::root:
        {
            // TODO: log impossible item type here
            break;
        }
        default:
        {
            abort();  // LCOV_EXCL_LINE  // Impossible
        }
    }
    if (item)
    {
        qf_.reportResult(item);
    }
    else
    {
        qf_.reportException(ResourceException("error"));  // TODO
    }
    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "MoveHandler.moc"
