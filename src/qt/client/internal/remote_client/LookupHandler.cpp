#include <unity/storage/qt/client/internal/remote_client/LookupHandler.h>

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

LookupHandler::LookupHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                             weak_ptr<Root> const& root)
    : watcher_(reply, this)
    , root_(root)
{
    assert(root.lock());
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &LookupHandler::finished);
    qf_.reportStarted();
}

QFuture<QVector<shared_ptr<Item>>> LookupHandler::future()
{
    return qf_.future();
}

void LookupHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<QList<storage::internal::ItemMetadata>> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    QVector<Item::SPtr> items;
    auto metadata = reply.value();
    for (auto const& md : metadata)
    {
        Item::SPtr item;
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
                continue;
                break;
            }
            default:
            {
                abort();  // LCOV_EXCL_LINE  // Impossible
            }
        }
        items.append(item);
    }
    if (items.isEmpty())
    {
        qf_.reportException(StorageException());  // TODO
    }
    else
    {
        qf_.reportResult(items);
    }
    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "LookupHandler.moc"
