#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/Root.h>

#include <boost/filesystem.hpp>

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

RootImpl::RootImpl(storage::internal::ItemMetadata const& md, weak_ptr<Account> const& account)
    : ItemBase(md.item_id, ItemType::root)
    , FolderBase(md.item_id, ItemType::root)
    , RootBase(md.item_id, account)
    , ItemImpl(md, ItemType::root)
    , FolderImpl(md, ItemType::root)
{
}

QFuture<QVector<Folder::SPtr>> RootImpl::parents() const
{
    QFutureInterface<QVector<Folder::SPtr>> qf;
    qf.reportResult(QVector<Folder::SPtr>());  // For the root, we return an empty vector.
    qf.reportFinished();
    return qf.future();
}

QVector<QString> RootImpl::parent_ids() const
{
    return QVector<QString>();  // For the root, we return an empty vector.
}

QFuture<void> RootImpl::destroy()
{
    // Cannot destroy root.
    QFutureInterface<void> qf;
    qf.reportException(StorageException());
    qf.reportFinished();
    return qf.future();
}

QFuture<int64_t> RootImpl::free_space_bytes() const
{
    return QFuture<int64_t>();
}

QFuture<int64_t> RootImpl::used_space_bytes() const
{
    return QFuture<int64_t>();
}

QFuture<Item::SPtr> RootImpl::get(QString native_identity) const
{
    return QFuture<Item::SPtr>();
}

Root::SPtr RootImpl::make_root(storage::internal::ItemMetadata const& md, std::weak_ptr<Account> const& account)
{
    assert(md.type == ItemType::root);
    assert(account.lock());

    auto impl = new RootImpl(md, account);
    Root::SPtr root(new Root(impl));
    impl->set_root(root);
    impl->set_public_instance(root);
    return root;
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
