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

#pragma once

#include <unity/storage/qt/Account.h>
#include <unity/storage/qt/Item.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDateTime>
#include <QVariantMap>
#include <QVector>
#pragma GCC diagnostic pop

#include <memory>

namespace unity
{
namespace storage
{
namespace internal
{

class ItemMetadata;

}

namespace qt
{
namespace internal
{

class AccountImpl;

class ItemImpl
{
public:
    ItemImpl();
    ItemImpl(ItemImpl const&) = default;
    ItemImpl(ItemImpl&&) = delete;
    ~ItemImpl() = default;
    ItemImpl& operator=(ItemImpl const&) = default;
    ItemImpl& operator=(ItemImpl&&) = delete;

    QString itemId() const;
    QString name() const;
    Account account() const;
    Item root() const;
    QString etag() const;
    Item::Type type() const;
    QVariantMap metadata() const;
    QDateTime lastModifiedTime() const;
    QVector<QString> parentIds() const;

    ItemListJob* parents() const;
    ItemJob* copy(Item const& newParent, QString const& newName) const;
    ItemJob* move(Item const& newParent, QString const& newName) const;
    VoidJob* deleteItem() const;
    Uploader* createUploader(ConflictPolicy policy, qint64 sizeInBytes) const;
    Downloader* createDownloader() const;
    ItemListJob* list() const;
    ItemListJob* lookup(QString const& name) const;
    ItemJob* createFolder(QString const& name) const;
    Uploader* createFile(QString const& name) const;
    ItemJob* get(QString const& itemId) const;
    IntJob* freeSpaceBytes() const;
    IntJob* usedSpaceBytes() const;

    bool operator==(ItemImpl const&) const;
    bool operator!=(ItemImpl const&) const;
    bool operator<(ItemImpl const&) const;
    bool operator<=(ItemImpl const&) const;
    bool operator>(ItemImpl const&) const;
    bool operator>=(ItemImpl const&) const;

    size_t hash() const;

    static Item make_item(storage::internal::ItemMetadata const& md, std::shared_ptr<AccountImpl> account);

private:
    bool is_valid_;
    QString item_id_;
    QString name_;
    Account account_;
    Item root_;
    QString etag_;
    Item::Type type_;
    QVariantMap metadata_;
    QDateTime last_modified_time_;
    QVector<QString> parent_ids_;

    friend class unity::storage::qt::Item;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
