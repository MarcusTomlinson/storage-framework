#include <unity/storage/qt/client/internal/remote_client/ItemImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/internal/remote_client/Handler.h>
#include <unity/storage/qt/client/internal/make_future.h>
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
        throw DeletedException();  // TODO
    }
    return md_.name;
}

QVariantMap ItemImpl::metadata() const
{
    if (deleted_)
    {
        throw DeletedException();  // TODO
    }
    // TODO: need to agree on metadata representation
    return QVariantMap();
}

QDateTime ItemImpl::last_modified_time() const
{
    if (deleted_)
    {
        throw DeletedException();  // TODO
    }
    // TODO: need to agree on metadata representation
    return QDateTime();
}

QFuture<shared_ptr<Item>> ItemImpl::copy(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Item>>(DeletedException());
    }

    auto process_copy_reply = [this](QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                                     QFutureInterface<std::shared_ptr<Item>>& qf)
    {
        if (reply.isError())
        {
            qDebug() << reply.error().message();  // TODO, remove this
            qf.reportException(StorageException());  // TODO
            qf.reportFinished();
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
            qf.reportResult(item);
        }
        else
        {
            qf.reportException(StorageException());
        }
        qf.reportFinished();
    };

    auto handler = new Handler<shared_ptr<Item>>(this,
                                                 provider().Copy(md_.item_id, new_parent->native_identity(), new_name),
                                                 process_copy_reply);
    return handler->future();
}

QFuture<shared_ptr<Item>> ItemImpl::move(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Item>>(DeletedException());
    }

    auto process_move_reply = [this](QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                                     QFutureInterface<std::shared_ptr<Item>>& qf)
    {
        if (reply.isError())
        {
            qDebug() << reply.error().message();  // TODO, remove this
            qf.reportException(StorageException());  // TODO
            qf.reportFinished();
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
            qf.reportResult(item);
        }
        else
        {
            qf.reportException(StorageException());  // TODO
        }
        qf.reportFinished();
    };

    auto handler = new Handler<shared_ptr<Item>>(this,
                                                 provider().Move(md_.item_id, new_parent->native_identity(), new_name),
                                                 process_move_reply);
    return handler->future();
}

QFuture<QVector<Folder::SPtr>> ItemImpl::parents() const
{
    if (deleted_)
    {
        return make_exceptional_future<QVector<shared_ptr<Folder>>>(DeletedException());
    }
    // TODO, need different metadata representation, affects xml
    return QFuture<QVector<Folder::SPtr>>();
}

QVector<QString> ItemImpl::parent_ids() const
{
    if (deleted_)
    {
        throw DeletedException();  // TODO
    }
    // TODO, need different metadata representation, affects xml
    return QVector<QString>();
}

QFuture<void> ItemImpl::delete_item()
{
    if (deleted_)
    {
        return make_exceptional_future(DeletedException());
    }

    auto process_delete_reply = [this](QDBusPendingReply<void> const& reply, QFutureInterface<void>& qf)
    {
        if (reply.isError())
        {
            qDebug() << reply.error().message();  // TODO: remove this
            qf.reportException(StorageException());  // TODO
        }
        deleted_ = true;
        qf.reportFinished();
    };

    auto handler = new Handler<void>(this, provider().Delete(md_.item_id), process_delete_reply);
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

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
