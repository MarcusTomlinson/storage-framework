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

#include <unity/storage/qt/internal/ItemImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/provider/metadata_keys.h>
#include <unity/storage/qt/internal/AccountImpl.h>
#include <unity/storage/qt/internal/ItemJobImpl.h>
#include <unity/storage/qt/internal/ItemListJobImpl.h>
#include <unity/storage/qt/internal/MultiItemJobImpl.h>
#include <unity/storage/qt/internal/RuntimeImpl.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>
#include <unity/storage/qt/internal/VoidJobImpl.h>
#include <unity/storage/qt/internal/validate.h>

#include <boost/functional/hash.hpp>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

ItemImpl::ItemImpl()
    : is_valid_(false)
{
    md_.type = storage::ItemType::file;
}

ItemImpl::ItemImpl(storage::internal::ItemMetadata const& md,
                   std::shared_ptr<AccountImpl> const& account)
    : is_valid_(true)
    , md_(md)
    , account_(account)
{
    assert(account);
}

QString ItemImpl::itemId() const
{
    return is_valid_ ? md_.item_id : "";
}

QString ItemImpl::name() const
{
    return is_valid_ ? md_.name : "";
}

Account ItemImpl::account() const
{
    return is_valid_ ? account_ : Account();
}

QString ItemImpl::etag() const
{
    return is_valid_ ? md_.etag : "";
}

Item::Type ItemImpl::type() const
{
    switch (md_.type)
    {
        case storage::ItemType::file:
            return Item::File;
        case storage::ItemType::folder:
            return Item::Folder;
        case storage::ItemType::root:
            return Item::Root;
        default:
            abort();  // LCOV_EXCL_LINE // Impossible
    }
}

QVariantMap ItemImpl::metadata() const
{
    // TODO: Need to agree on metadata representation.
    return is_valid_ ? QVariantMap() : QVariantMap();
}

QDateTime ItemImpl::lastModifiedTime() const
{
    return is_valid_ ? QDateTime::fromString(md_.metadata.value(provider::LAST_MODIFIED_TIME).toString(), Qt::ISODate)
                     : QDateTime();
}

QList<QString> ItemImpl::parentIds() const
{
    if (!is_valid_ || md_.type == storage::ItemType::root)
    {
        return QList<QString>();
    }
    return md_.parent_ids;
}

ItemListJob* ItemImpl::parents() const
{
    QString const method = "Item::parents()";

    if (!is_valid_)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot create job from invalid item");
        return ListJobImplBase::make_job(e);
    }
    auto runtime = account_->runtime();
    if (!runtime || !runtime->isValid())
    {
        auto e = StorageErrorImpl::runtime_destroyed_error(method + ": Runtime was destroyed previously");
        return ListJobImplBase::make_job(e);
    }

    if (md_.type == storage::ItemType::root)
    {
        return ListJobImplBase::make_empty_job();  // Root has no parents.
    }

    assert(!md_.parent_ids.isEmpty());

    QList<QDBusPendingReply<storage::internal::ItemMetadata>> replies;
    for (auto const& id : md_.parent_ids)
    {
        auto reply = account_->provider()->Metadata(id);
        replies.append(reply);
    }

    auto validate = [method](storage::internal::ItemMetadata const& md)
    {
        if (md.type == ItemType::file)
        {
            QString msg = method + ": provider returned a file as a parent";
            qCritical() << msg;
            throw StorageErrorImpl::local_comms_error(msg);
        }
    };

    return MultiItemJobImpl::make_job(account_, method, replies, validate);
}

ItemJob* ItemImpl::copy(Item const& newParent, QString const& newName) const
{
    return nullptr;  // TODO
}

ItemJob* ItemImpl::move(Item const& newParent, QString const& newName) const
{
    return nullptr;  // TODO
}

VoidJob* ItemImpl::deleteItem() const
{
    QString const method = "Item::deleteItem()";

    if (!is_valid_)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot create job from invalid item");
        return VoidJobImpl::make_job(e);
    }
    auto runtime = account_->runtime();
    if (!runtime || !runtime->isValid())
    {
        auto e = StorageErrorImpl::runtime_destroyed_error(method + ": Runtime was destroyed previously");
        return VoidJobImpl::make_job(e);
    }
    if (md_.type == storage::ItemType::root)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot delete root");
        return VoidJobImpl::make_job(e);
    }

    auto reply = account_->provider()->Delete(md_.item_id);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return VoidJobImpl::make_job(This, method, reply);
}

Uploader* ItemImpl::createUploader(Item::ConflictPolicy policy, qint64 sizeInBytes) const
{
    return nullptr;  // TODO
}

Downloader* ItemImpl::createDownloader() const
{
    return nullptr;  // TODO
}

ItemListJob* ItemImpl::list() const
{
    return nullptr;  // TODO
}

ItemListJob* ItemImpl::lookup(QString const& name) const
{
    return nullptr;  // TODO
}

ItemJob* ItemImpl::createFolder(QString const& name) const
{
    return nullptr;  // TODO
}

Uploader* ItemImpl::createFile(QString const& name) const
{
    return nullptr;  // TODO
}

IntJob* ItemImpl::freeSpaceBytes() const
{
    return nullptr;  // TODO
}

IntJob* ItemImpl::usedSpaceBytes() const
{
    return nullptr;  // TODO
}

bool ItemImpl::operator==(ItemImpl const& other) const
{
    if (is_valid_)
    {
        return other.is_valid_
               && *account_ == *other.account_
               && md_.item_id == other.md_.item_id;
    }
    return !other.is_valid_;
}

bool ItemImpl::operator!=(ItemImpl const& other) const
{
    return !operator==(other);
}

bool ItemImpl::operator<(ItemImpl const& other) const
{
    if (!is_valid_)
    {
        return other.is_valid_;
    }
    if (is_valid_ && !other.is_valid_)
    {
        return false;
    }
    assert(is_valid_ && other.is_valid_);
    if (*account_ < *other.account_)
    {
        return true;
    }
    if (*account_ > *other.account_)
    {
        return false;
    }
    return md_.item_id < other.md_.item_id;
}

bool ItemImpl::operator<=(ItemImpl const& other) const
{
    return operator<(other) or operator==(other);
}

bool ItemImpl::operator>(ItemImpl const& other) const
{
    return !operator<=(other);
}

bool ItemImpl::operator>=(ItemImpl const& other) const
{
    return !operator<(other);
}

size_t ItemImpl::hash() const
{
    if (!is_valid_)
    {
        return 0;
    }
    size_t hash = 0;
    boost::hash_combine(hash, account_->hash());
    boost::hash_combine(hash, qHash(md_.item_id));
    return hash;
}

Item ItemImpl::make_item(QString const& method,
                         storage::internal::ItemMetadata const& md,
                         std::shared_ptr<AccountImpl> const& account)
{
    validate(method, md);  // Throws if no good.
    auto p = make_shared<ItemImpl>(md, account);
    return Item(p);
}

shared_ptr<RuntimeImpl> ItemImpl::runtime() const
{
    return account_->runtime();
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
