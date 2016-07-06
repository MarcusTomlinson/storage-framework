#include <unity/storage/qt/client/internal/remote_client/FolderImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/Folder.h>
#include <unity/storage/qt/client/internal/make_future.h>
#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>
#include <unity/storage/qt/client/internal/remote_client/CreateFileHandler.h>
#include <unity/storage/qt/client/internal/remote_client/CreateFolderHandler.h>
#include <unity/storage/qt/client/internal/remote_client/ListHandler.h>
#include <unity/storage/qt/client/internal/remote_client/LookupHandler.h>

#include <QtConcurrent>

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

FolderImpl::FolderImpl(storage::internal::ItemMetadata const& md)
    : ItemBase(md.item_id, ItemType::folder)
    , FolderBase(md.item_id, ItemType::folder)
    , ItemImpl(md, ItemType::folder)
{
}

FolderImpl::FolderImpl(storage::internal::ItemMetadata const& md, ItemType type)
    : ItemBase(md.item_id, type)
    , FolderBase(md.item_id, type)
    , ItemImpl(md, type)
{
}

QFuture<QVector<shared_ptr<Item>>> FolderImpl::list() const
{
    if (deleted_)
    {
        return make_exceptional_future<QVector<shared_ptr<Item>>>(DeletedException());
    }
    QFutureInterface<QVector<shared_ptr<Item>>> qf;
    qf.reportStarted();
    auto handler = new ListHandler(provider().List(md_.item_id, ""), root_, md_.item_id, provider(), qf);
    return handler->future();
}

QFuture<QVector<shared_ptr<Item>>> FolderImpl::lookup(QString const& name) const
{
    if (deleted_)
    {
        return make_exceptional_future<QVector<shared_ptr<Item>>>(DeletedException());
    }
    auto handler = new LookupHandler(provider().Lookup(md_.item_id, name), root_);
    return handler->future();
}

QFuture<shared_ptr<Folder>> FolderImpl::create_folder(QString const& name)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Folder>>(DeletedException());
    }
    auto handler = new CreateFolderHandler(provider().CreateFolder(md_.item_id, name), root_);
    return handler->future();
}

QFuture<shared_ptr<Uploader>> FolderImpl::create_file(QString const& name)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Uploader>>(DeletedException());
    }
    auto handler = new CreateFileHandler(provider().CreateFile(md_.item_id, name, "application/octet-stream", false),
                                         root_,
                                         provider());
    return handler->future();
}

shared_ptr<Folder> FolderImpl::make_folder(storage::internal::ItemMetadata const& md, weak_ptr<Root> root)
{
    assert(md.type == ItemType::folder);
    assert(root.lock());

    auto impl = new FolderImpl(md);
    shared_ptr<Folder> folder(new Folder(impl));
    impl->set_root(root);
    impl->set_public_instance(folder);
    return folder;
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "FolderImpl.moc"
