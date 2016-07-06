#include <unity/storage/qt/client/internal/remote_client/ItemImpl.h>

#include "ProviderInterface.h"
// TODO: check this include list
#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>
#include <unity/storage/qt/client/internal/remote_client/CopyHandler.h>
#include <unity/storage/qt/client/internal/remote_client/DeleteHandler.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/internal/remote_client/MoveHandler.h>
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
    , md_(md)
{
}

QString ItemImpl::name() const
{
    if (deleted_)
    {
        throw deleted_ex("Item::name()");
    }
    return md_.name;
}

QVariantMap ItemImpl::metadata() const
{
    if (deleted_)
    {
        throw deleted_ex("Item::metadata()");
    }
    // TODO: need to agree on metadata representation
    return QVariantMap();
}

QDateTime ItemImpl::last_modified_time() const
{
    if (deleted_)
    {
        throw deleted_ex("Item::last_modified_time()");
    }
    // TODO: need to agree on metadata representation
    return QDateTime();
}

QFuture<shared_ptr<Item>> ItemImpl::copy(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    if (deleted_)
    {
        QFutureInterface<shared_ptr<Item>> qf;
        qf.reportException(deleted_ex("Item::copy()"));
        qf.reportFinished();
        return qf.future();
    }
    auto handler = new CopyHandler(provider().Copy(md_.item_id, new_parent->native_identity(), new_name), root_);
    return handler->future();
}

QFuture<shared_ptr<Item>> ItemImpl::move(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    if (deleted_)
    {
        QFutureInterface<shared_ptr<Item>> qf;
        qf.reportException(deleted_ex("Item::move()"));
        qf.reportFinished();
        return qf.future();
    }
    auto handler = new MoveHandler(provider().Move(md_.item_id, new_parent->native_identity(), new_name), root_);
    return handler->future();
}

QFuture<QVector<Folder::SPtr>> ItemImpl::parents() const
{
    if (deleted_)
    {
        QFutureInterface<QVector<shared_ptr<Folder>>> qf;
        qf.reportException(deleted_ex("Item::parents()"));
        qf.reportFinished();
        return qf.future();
    }
    // TODO, need different metadata representation, affects xml
    return QFuture<QVector<Folder::SPtr>>();
}

QVector<QString> ItemImpl::parent_ids() const
{
    if (deleted_)
    {
        throw deleted_ex("Item::parent_ids()");
    }
    // TODO, need different metadata representation, affects xml
    return QVector<QString>();
}

QFuture<void> ItemImpl::delete_item()
{
    if (deleted_)
    {
        QFutureInterface<void> qf;
        qf.reportException(deleted_ex("Item::delete_item()"));
        qf.reportFinished();
        return qf.future();
    }
    auto handler = new DeleteHandler(provider().Delete(md_.item_id),
                                     dynamic_pointer_cast<ItemImpl>(shared_from_this()));
    return handler->future();
}

bool ItemImpl::equal_to(ItemBase const& other) const noexcept
{
    auto other_impl = dynamic_cast<ItemImpl const*>(&other);
    assert(other_impl);
    if (this == other_impl)
    {
        return true;
    }
    if (deleted_ || other_impl->deleted_)
    {
        return false;
    }
    return identity_ == other_impl->identity_;
}

ProviderInterface& ItemImpl::provider() const noexcept
{
    auto root_impl = dynamic_pointer_cast<RootImpl>(root_.lock()->p_);
    auto account_impl = dynamic_pointer_cast<AccountImpl>(root_impl->account_.lock()->p_);
    return account_impl->provider();
}

DeletedException ItemImpl::deleted_ex(QString const& method) const noexcept
{
    QString msg = method + ": " + identity_ + " was deleted previously";
    return DeletedException(msg, identity_, md_.name);
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
