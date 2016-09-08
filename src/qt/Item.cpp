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

#include <unity/storage/qt/Item.h>
#include <unity/storage/qt/internal/ItemImpl.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{

Item::Item()
    : p_(make_shared<internal::ItemImpl>())
{
}

Item::Item(Item const& other)
    : p_(make_shared<internal::ItemImpl>(*other.p_))
{
}

Item::Item(Item&& other)
    : p_(make_shared<internal::ItemImpl>())
{
    swap(p_, other.p_);
}

Item::~Item() = default;

Item& Item::operator=(Item const& other)
{
    if (this == &other)
    {
        return *this;
    }
    *p_ = *other.p_;
    return *this;
}

Item& Item::operator=(Item&& other)
{
    p_->is_valid_ = false;
    swap(p_, other.p_);
    return *this;
}

bool Item::isValid() const
{
    return p_->is_valid_;
}

QString Item::itemId() const
{
    return p_->itemId();
}

QString Item::name() const
{
    return p_->name();
}

Account Item::account() const
{
    return p_->account();
}

Item Item::root() const
{

    return p_->root();
}

QString Item::etag() const
{
    return p_->etag();
}

Item::Type Item::type() const
{
    return p_->type();
}

QVariantMap Item::metadata() const
{
    return p_->metadata();
}

QDateTime Item::lastModifiedTime() const
{
    return p_->lastModifiedTime();
}

QVector<QString> Item::parentIds() const
{
    return p_->parentIds();
}

ItemListJob* Item::parents() const
{
    return p_->parents();
}

ItemJob* Item::copy(Item const& newParent, QString const& newName) const
{
    return p_->copy(newParent, newName);
}

ItemJob* Item::move(Item const& newParent, QString const& newName) const
{
    return p_->move(newParent, newName);
}

VoidJob* Item::deleteItem() const
{
    return p_->deleteItem();
}

Uploader* Item::createUploader(ConflictPolicy policy, qint64 sizeInBytes) const
{
    return p_->createUploader(policy, sizeInBytes);
}

Downloader* Item::createDownloader() const
{
    return p_->createDownloader();
}

ItemListJob* Item::list() const
{
    return p_->list();
}

ItemListJob* Item::lookup(QString const& name) const
{
    return p_->lookup(name);
}

ItemJob* Item::createFolder(QString const& name) const
{
    return p_->createFolder(name);
}

Uploader* Item::createFile(QString const& name) const
{
    return p_->createFile(name);
}

ItemJob* Item::get(QString const& itemId) const
{
    return p_->get(itemId);
}

IntJob* Item::freeSpaceBytes() const
{
    return p_->freeSpaceBytes();
}

IntJob* Item::usedSpaceBytes() const
{
    return p_->usedSpaceBytes();
}

bool Item::operator==(Item const& other) const
{
    return p_->operator==(*other.p_);
}

bool Item::operator!=(Item const& other) const
{
    return p_->operator!=(*other.p_);
}

bool Item::operator<(Item const& other) const
{
    return p_->operator<(*other.p_);
}

bool Item::operator<=(Item const& other) const
{
    return p_->operator<=(*other.p_);
}

bool Item::operator>(Item const& other) const
{
    return p_->operator>(*other.p_);
}

bool Item::operator>=(Item const& other) const
{
    return p_->operator>=(*other.p_);
}

}  // namespace qt
}  // namespace storage
}  // namespace unity
