#include <unity/storage/qt/client/internal/remote_client/MetadataHandler.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/Root.h>
#include <unity/storage/qt/client/internal/remote_client/dbusmarshal.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>

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

MetadataHandler::MetadataHandler(QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                                 weak_ptr<Root> const& root)
    : watcher_(reply, this)
    , root_(root)
{
    assert(root.lock());
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &MetadataHandler::finished);
    qf_.reportStarted();
}

QFuture<shared_ptr<Item>> MetadataHandler::future()
{
    return qf_.future();
}

void MetadataHandler::finished(QDBusPendingCallWatcher* call)
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

    auto md = reply.value();
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
            item = RootImpl::make_root(md, dynamic_pointer_cast<RootImpl>(root_.lock()->p_)->account_);
            break;
        }
        default:
        {
            abort();  // LCOV_EXCL_LINE  // Impossible
        }
    }
    qf_.reportResult(item);
    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "MetadataHandler.moc"
