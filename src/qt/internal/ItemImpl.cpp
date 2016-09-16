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

#include <unity/storage/provider/metadata_keys.h>
#include <unity/storage/qt/internal/validate.h>

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

#if 0
Item ItemImpl::root() const
{
    return is_valid_ ? root_ : Item();
}
#endif

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

QVector<QString> ItemImpl::parentIds() const
{
    return is_valid_ ? md_.parent_ids : QVector<QString>();
}

ItemListJob* ItemImpl::parents() const
{
    return nullptr;  // TODO
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
    return nullptr;  // TODO
}

Uploader* ItemImpl::createUploader(ConflictPolicy policy, qint64 sizeInBytes) const
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

ItemJob* ItemImpl::get(QString const& itemId) const
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
        return other.is_valid_ && md_.item_id == other.md_.item_id;
    }
    return !other.is_valid_;
}

bool ItemImpl::operator!=(ItemImpl const& other) const
{
    return !operator==(other);
}

bool ItemImpl::operator<(ItemImpl const& other) const
{
    if (is_valid_)
    {
        return other.is_valid_ && md_.item_id < other.md_.item_id;
    }
    return other.is_valid_;
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
    return qHash(md_.item_id);
}

Item ItemImpl::make_item(QString const& method,
                         storage::internal::ItemMetadata const& md,
                         std::shared_ptr<AccountImpl> const& account)
{
    validate(method, md);  // Throws if no good.
    auto p = make_shared<ItemImpl>(md, account);
    return Item(p);
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
