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

#include <unity/storage/qt/internal/ItemImpl>

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
    , type_(Item::Type::File)
{
}

QString ItemImpl::itemId() const
{
    return is_valid_ ? item_id_ : "";
}

QString ItemImpl::name() const
{
    return is_valid_ ? name_ : "";
}

Account ItemImpl::account() const
{
    return is_valid_ ? account_ : Account();
}

Item ItemImpl::root() const
{
    return is_valid_ ? root_ : Item();
}

QString ItemImpl::etag() const
{
    return is_valid_ ? etag_ : "";
}

Item::Type ItemImpl::type() const
{
    return type_;
}

QVariantMap ItemImpl::metadata() const
{
    return is_valid_ ? metadata_ : QVariantMap();
}

QDateTime ItemImpl::lastModifiedTime() const
{
    return is_valid_ ? last_modified_time_ : QDateTime();
}

QVector<QString> ItemImpl::parentIds() const
{
    return is_valid_ ? parent_ids_ : QVector<QString>();
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

UploadJob* ItemImpl::createUploader(ConflictPolicy policy, qint64 sizeInBytes) const
{
    return nullptr;  // TODO
}

DownloadJob* ItemImpl::createDownloader() const
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

UploadJob* ItemImpl::createFile(QString const& name) const
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
        return other.is_valid_ && item_id_ == other.item_id_;
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
        return other.is_valid_ && item_id_ < other.item_id_;
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

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
