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
#include <unity/storage/common.h>
#include <unity/storage/qt/internal/DownloaderImpl.h>
#include <unity/storage/qt/internal/ItemJobImpl.h>
#include <unity/storage/qt/internal/ItemListJobImpl.h>
#include <unity/storage/qt/internal/MultiItemJobImpl.h>
#include <unity/storage/qt/internal/MultiItemListJobImpl.h>
#include <unity/storage/qt/internal/UploaderImpl.h>
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
                   std::shared_ptr<AccountImpl> const& account_impl)
    : is_valid_(true)
    , md_(md)
    , account_impl_(account_impl)
{
    assert(account_impl);
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
    return is_valid_ ? account_impl_ : Account();
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
            return Item::Type::File;
        case storage::ItemType::folder:
            return Item::Type::Folder;
        case storage::ItemType::root:
            return Item::Type::Root;
        default:
            abort();  // Impossible.  // LCOV_EXCL_LINE
    }
}

QVariantMap ItemImpl::metadata() const
{
    return is_valid_ ? md_.metadata : QVariantMap();
}

qint64 ItemImpl::sizeInBytes() const
{
    if (!is_valid_ || md_.type != ItemType::file)
    {
        return 0;
    }
    auto variant = md_.metadata.value(metadata::SIZE_IN_BYTES);
    assert(variant.isValid());
    return variant.toLongLong();
}

QDateTime ItemImpl::lastModifiedTime() const
{
    return is_valid_ ? QDateTime::fromString(md_.metadata.value(metadata::LAST_MODIFIED_TIME).toString(), Qt::ISODate)
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

ItemListJob* ItemImpl::parents(QStringList const& keys) const
{
    QString const method = "Item::parents()";

    auto invalid_job = check_invalid_or_destroyed<ItemListJobImpl>(method);
    if (invalid_job)
    {
        return invalid_job;
    }

    if (md_.type == storage::ItemType::root)
    {
        return ListJobImplBase::make_empty_job();  // Root has no parents.
    }

    assert(!md_.parent_ids.isEmpty());

    QList<QDBusPendingReply<storage::internal::ItemMetadata>> replies;
    for (auto const& id : md_.parent_ids)
    {
        auto reply = account_impl_->provider()->Metadata(id, keys);
        replies.append(reply);
    }

    auto validate = [method](storage::internal::ItemMetadata const& md)
    {
        if (md.type == ItemType::file)
        {
            QString msg = method + ": provider returned a file as a parent (id = " + md.item_id + ")";
            qCritical().noquote() << msg;
            throw StorageErrorImpl::local_comms_error(msg);
        }
    };

    return MultiItemJobImpl::make_job(account_impl_, method, replies, validate);
}

ItemJob* ItemImpl::copy(Item const& newParent, QString const& newName, QStringList const& keys) const
{
    QString const method = "Item::copy()";

    auto invalid_job = check_copy_move_precondition<ItemJobImpl>(method, newParent, newName);
    if (invalid_job)
    {
        return invalid_job;
    }

    auto validate = [this, method](storage::internal::ItemMetadata const& md)
    {
        if ((md_.type == ItemType::file && md.type != ItemType::file)
            ||
            (md_.type != ItemType::file && md.type == ItemType::file))
        {
            QString msg = method + "provider error: source and target item type differ (source id = " +
                          md_.item_id + ", target id = " + md.item_id + ")";
            qCritical().noquote() << msg;
            throw StorageErrorImpl::local_comms_error(msg);
        }
    };

    auto reply = account_impl_->provider()->Copy(md_.item_id, newParent.itemId(), newName, keys);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return ItemJobImpl::make_job(This, method, reply, validate);
}

ItemJob* ItemImpl::move(Item const& newParent, QString const& newName, QStringList const& keys) const
{
    QString const method = "Item::move()";

    auto invalid_job = check_copy_move_precondition<ItemJobImpl>(method, newParent, newName);
    if (invalid_job)
    {
        return invalid_job;
    }

    auto validate = [this, method](storage::internal::ItemMetadata const& md)
    {
        if (md.type == ItemType::root)
        {
            QString msg = method + ": impossible root item returned by provider (id = " + md.item_id + ")";
            qCritical().noquote() << msg;
            throw StorageErrorImpl::local_comms_error(msg);
        }
        if ((md_.type == ItemType::file && md.type != ItemType::file)
            ||
            (md_.type != ItemType::file && md.type == ItemType::file))
        {
            QString msg = method + ": provider error: source and target item type differ (source id = " +
                          md_.item_id + ", target id = " + md.item_id + ")";
            qCritical().noquote() << msg;
            throw StorageErrorImpl::local_comms_error(msg);
        }
    };

    auto reply = account_impl_->provider()->Move(md_.item_id, newParent.itemId(), newName, keys);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return ItemJobImpl::make_job(This, method, reply, validate);
}

VoidJob* ItemImpl::deleteItem() const
{
    QString const method = "Item::deleteItem()";

    auto invalid_job = check_invalid_or_destroyed<VoidJobImpl>(method);
    if (invalid_job)
    {
        return invalid_job;
    }
    if (md_.type == storage::ItemType::root)
    {
        auto e = StorageErrorImpl::permission_error(method + ": cannot delete root");
        return VoidJobImpl::make_job(e);
    }

    auto reply = account_impl_->provider()->Delete(md_.item_id);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return VoidJobImpl::make_job(This, method, reply);
}

Uploader* ItemImpl::createUploader(Item::ConflictPolicy policy, qint64 sizeInBytes, QStringList const& keys) const
{
    QString const method = "Item::createUploader()";

    auto invalid_job = check_invalid_or_destroyed<UploaderImpl>(method);
    if (invalid_job)
    {
        return invalid_job;
    }
    if (md_.type != storage::ItemType::file)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot upload to a folder");
        return UploaderImpl::make_job(e);
    }
    if (sizeInBytes < 0)
    {
        auto e = StorageErrorImpl::invalid_argument_error(method + ": size must be >= 0");
        return UploaderImpl::make_job(e);
    }

    auto validate = [method](storage::internal::ItemMetadata const& md)
    {
        if (md.type != storage::ItemType::file)
        {
            QString msg = method + ": impossible folder item returned by provider (id = " + md.item_id + ")";
            qCritical().noquote() << msg;
            throw StorageErrorImpl::local_comms_error(msg);
        }
    };

    auto etag = policy == Item::ConflictPolicy::IgnoreConflict ? "" : md_.etag;
    auto reply = account_impl_->provider()->Update(md_.item_id, sizeInBytes, etag, keys);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return UploaderImpl::make_job(This, method, reply, validate, policy, sizeInBytes);
}

Downloader* ItemImpl::createDownloader(Item::ConflictPolicy policy) const
{
    QString const method = "Item::createDownloader()";

    auto invalid_job = check_invalid_or_destroyed<DownloaderImpl>(method);
    if (invalid_job)
    {
        return invalid_job;
    }
    if (md_.type != storage::ItemType::file)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot download a folder");
        return DownloaderImpl::make_job(e);
    }

    auto etag = policy == Item::ConflictPolicy::IgnoreConflict ? "" : md_.etag;
    auto reply = account_impl_->provider()->Download(md_.item_id, etag);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return DownloaderImpl::make_job(This, method, reply);
}

ItemListJob* ItemImpl::list(QStringList const& keys) const
{
    QString const method = "Item::list()";

    auto invalid_job = check_invalid_or_destroyed<MultiItemListJobImpl>(method);
    if (invalid_job)
    {
        return invalid_job;
    }
    if (md_.type == storage::ItemType::file)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot perform list on a file");
        return ItemListJobImpl::make_job(e);
    }

    auto validate = [method](storage::internal::ItemMetadata const& md)
    {
        if (md.type == storage::ItemType::root)
        {
            QString msg = method + ": impossible root item returned by provider (id = " + md.item_id + ")";
            qCritical().noquote() << msg;
            throw StorageErrorImpl::local_comms_error(msg);
        }
    };

    auto fetch_next = [this, keys](QString const& page_token)
    {
        return account_impl_->provider()->List(md_.item_id, page_token, keys);
    };

    auto reply = account_impl_->provider()->List(md_.item_id, "", keys);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return MultiItemListJobImpl::make_job(This, method, reply, validate, fetch_next);
}

ItemListJob* ItemImpl::lookup(QString const& name, QStringList const& keys) const
{
    QString const method = "Item::lookup()";

    auto invalid_job = check_invalid_or_destroyed<ItemListJobImpl>(method);
    if (invalid_job)
    {
        return invalid_job;
    }
    if (md_.type == storage::ItemType::file)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot perform lookup on a file");
        return ItemListJobImpl::make_job(e);
    }

    auto validate = [](storage::internal::ItemMetadata const&)
    {
    };

    auto reply = account_impl_->provider()->Lookup(md_.item_id, name, keys);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return ItemListJobImpl::make_job(This, method, reply, validate);
}

ItemJob* ItemImpl::createFolder(QString const& name, QStringList const& keys) const
{
    QString const method = "Item::createFolder()";

    auto invalid_job = check_invalid_or_destroyed<ItemJobImpl>(method);
    if (invalid_job)
    {
        return invalid_job;
    }
    if (md_.type == storage::ItemType::file)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot create a folder with a file as the parent");
        return ItemJobImpl::make_job(e);
    }

    auto validate = [method](storage::internal::ItemMetadata const& md)
    {
        if (md.type != storage::ItemType::file)
        {
            return;
        }
        QString msg = method + ": impossible file item returned by provider (id = " + md.item_id + ")";
        qCritical().noquote() << msg;
        throw StorageErrorImpl::local_comms_error(msg);
    };

    auto reply = account_impl_->provider()->CreateFolder(md_.item_id, name, keys);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return ItemJobImpl::make_job(This, method, reply, validate);
}

Uploader* ItemImpl::createFile(QString const& name,
                               Item::ConflictPolicy policy,
                               qint64 sizeInBytes,
                               QString const& contentType,
                               QStringList const& keys) const
{
    QString const method = "Item::createFile()";

    auto invalid_job = check_invalid_or_destroyed<UploaderImpl>(method);
    if (invalid_job)
    {
        return invalid_job;
    }
    if (md_.type == storage::ItemType::file)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot create a file with a file as the parent");
        return UploaderImpl::make_job(e);
    }
    if (sizeInBytes < 0)
    {
        auto e = StorageErrorImpl::invalid_argument_error(method + ": size must be >= 0");
        return UploaderImpl::make_job(e);
    }
    if (name.isEmpty())
    {
        auto e = StorageErrorImpl::invalid_argument_error(method + ": name cannot be empty");
        return UploaderImpl::make_job(e);
    }
    // contentType can be empty, so not checked here.

    auto validate = [method](storage::internal::ItemMetadata const& md)
    {
        if (md.type != storage::ItemType::file)
        {
            QString msg = method + ": impossible folder item returned by provider (id = " + md.item_id + ")";
            qCritical().noquote() << msg;
            throw StorageErrorImpl::local_comms_error(msg);
        }
    };

    bool allow_overwrite = policy == Item::ConflictPolicy::IgnoreConflict;
    auto reply = account_impl_->provider()->CreateFile(md_.item_id, name, sizeInBytes,
                                                       contentType, allow_overwrite, keys);
    auto This = const_pointer_cast<ItemImpl>(shared_from_this());
    return UploaderImpl::make_job(This, method, reply, validate, policy, sizeInBytes);
}

bool ItemImpl::operator==(ItemImpl const& other) const
{
    if (is_valid_)
    {
        return other.is_valid_
               && *account_impl_ == *other.account_impl_
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
    if (*account_impl_ < *other.account_impl_)
    {
        return true;
    }
    if (*account_impl_ > *other.account_impl_)
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
    boost::hash_combine(hash, account_impl_->hash());
    boost::hash_combine(hash, qHash(md_.item_id));
    return hash;
}

Item ItemImpl::make_item(QString const& method,
                         storage::internal::ItemMetadata const& md,
                         std::shared_ptr<AccountImpl> const& account_impl)
{
    validate(method, md);  // Throws if no good.
    auto p = make_shared<ItemImpl>(md, account_impl);
    return Item(p);
}

shared_ptr<RuntimeImpl> ItemImpl::runtime_impl() const
{
    return account_impl_->runtime_impl();
}

shared_ptr<AccountImpl> ItemImpl::account_impl() const
{
    return account_impl_;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
