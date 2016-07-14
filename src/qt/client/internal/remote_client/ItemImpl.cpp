/*
 * Copyright (C) 2016 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/storage/qt/client/internal/remote_client/ItemImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Account.h>
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

QString ItemImpl::etag() const
{
    if (deleted_)
    {
        throw DeletedException();  // TODO
    }
    return md_.etag;
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

    auto reply = provider().Copy(md_.item_id, new_parent->native_identity(), new_name);
    auto process_reply = [this](decltype(reply) const& reply, QFutureInterface<std::shared_ptr<Item>>& qf)
    {
        auto md = reply.value();
        if (md.type == ItemType::root)
        {
            // TODO: log server error here
            return make_exceptional_future(qf, StorageException());  // TODO
        }
        return make_ready_future(qf, ItemImpl::make_item(md, root_));
    };

    auto handler = new Handler<shared_ptr<Item>>(this, reply, process_reply);
    return handler->future();
}

QFuture<shared_ptr<Item>> ItemImpl::move(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Item>>(DeletedException());
    }

    auto reply = provider().Move(md_.item_id, new_parent->native_identity(), new_name);
    auto process_reply = [this](decltype(reply) const& reply, QFutureInterface<std::shared_ptr<Item>>& qf)
    {
        auto md = reply.value();
        if (md.type == ItemType::root)
        {
            // TODO: log server error here
            return make_exceptional_future(qf, StorageException());  // TODO
        }
        return make_ready_future(qf, ItemImpl::make_item(md, root_));
    };

    auto handler = new Handler<shared_ptr<Item>>(this, reply, process_reply);
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

    auto reply = provider().Delete(md_.item_id);
    auto process_reply = [this](decltype(reply) const&, QFutureInterface<void>& qf)
    {
        deleted_ = true;
        make_ready_future(qf);
    };

    auto handler = new Handler<void>(this, reply, process_reply);
    return handler->future();
}

QDateTime ItemImpl::creation_time() const
{
    if (deleted_)
    {
        throw DeletedException();  // TODO
    }
    // TODO: need to agree on metadata representation
    return QDateTime();
}

MetadataMap ItemImpl::native_metadata() const
{
    if (deleted_)
    {
        throw DeletedException();  // TODO
    }
    // TODO: need to agree on metadata representation
    return MetadataMap();
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

shared_ptr<Item> ItemImpl::make_item(storage::internal::ItemMetadata const& md, std::weak_ptr<Root> root)
{
    assert(md.type == ItemType::file || md.type == ItemType::folder);

    shared_ptr<Item> item;
    switch (md.type)
    {
        case ItemType::file:
        {
            item = FileImpl::make_file(md, root);
            break;
        }
        case ItemType::folder:
        {
            item = FolderImpl::make_folder(md, root);
            break;
        }
        default:
        {
            abort();  // LCOV_EXCL_LINE  // Impossible
        }
    }
    assert(item);
    return item;
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
