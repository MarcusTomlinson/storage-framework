#include <unity/storage/qt/client/internal/remote_client/ItemImpl.h>

#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>

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

ItemImpl::ItemImpl(storage::internal::ItemMetadata const& md, ItemType type)
    : ItemBase(md.item_id, type)
{
}

QString ItemImpl::name() const
{
    return md_.name;
}

QVariantMap ItemImpl::metadata() const
{
    return QVariantMap();
}

QDateTime ItemImpl::last_modified_time() const
{
    return QDateTime();
}

QFuture<shared_ptr<Item>> ItemImpl::copy(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    return QFuture<shared_ptr<Item>>();
}

QFuture<shared_ptr<Item>> ItemImpl::move(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    return QFuture<shared_ptr<Item>>();
}

QFuture<QVector<Folder::SPtr>> ItemImpl::parents() const
{
    return QFuture<QVector<Folder::SPtr>>();
}

QVector<QString> ItemImpl::parent_ids() const
{
    return QVector<QString>();
}

QFuture<void> ItemImpl::destroy()
{
    return QFuture<void>();
}

bool ItemImpl::equal_to(ItemBase const& other) const noexcept
{
    return false;
}

ProviderInterface& ItemImpl::provider() const noexcept
{
    auto root_impl = dynamic_pointer_cast<RootImpl>(root_.lock()->p_);
    auto account_impl = dynamic_pointer_cast<AccountImpl>(root_impl->account_.lock()->p_);
    return account_impl->provider();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
