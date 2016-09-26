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

#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/Account.h>
#include <unity/storage/qt/Item.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class AccountImpl;
class RuntimeImpl;

class ItemImpl : public std::enable_shared_from_this<ItemImpl>
{
public:
    ItemImpl();
    ItemImpl(storage::internal::ItemMetadata const& md,
             std::shared_ptr<AccountImpl> const& account);
    ItemImpl(ItemImpl const&) = default;
    ItemImpl(ItemImpl&&) = delete;
    ~ItemImpl() = default;
    ItemImpl& operator=(ItemImpl const&) = default;
    ItemImpl& operator=(ItemImpl&&) = delete;

    QString itemId() const;
    QString name() const;
    Account account() const;
    QString etag() const;
    Item::Type type() const;
    QVariantMap metadata() const;
    QDateTime lastModifiedTime() const;
    QVector<QString> parentIds() const;

    ItemListJob* parents() const;
    ItemJob* copy(Item const& newParent, QString const& newName) const;
    ItemJob* move(Item const& newParent, QString const& newName) const;
    VoidJob* deleteItem() const;
    Uploader* createUploader(Item::ConflictPolicy policy, qint64 sizeInBytes) const;
    Downloader* createDownloader() const;
    ItemListJob* list() const;
    ItemListJob* lookup(QString const& name) const;
    ItemJob* createFolder(QString const& name) const;
    Uploader* createFile(QString const& name) const;
    IntJob* freeSpaceBytes() const;
    IntJob* usedSpaceBytes() const;

    bool operator==(ItemImpl const&) const;
    bool operator!=(ItemImpl const&) const;
    bool operator<(ItemImpl const&) const;
    bool operator<=(ItemImpl const&) const;
    bool operator>(ItemImpl const&) const;
    bool operator>=(ItemImpl const&) const;

    size_t hash() const;

    static Item make_item(QString const& method,
                          storage::internal::ItemMetadata const& md,
                          std::shared_ptr<AccountImpl> const& account);

    std::shared_ptr<RuntimeImpl> runtime() const;

private:
    //std::shared_ptr<RuntimeImpl> get_runtime(QString const& method) const;

    bool is_valid_;
    storage::internal::ItemMetadata md_;
    std::shared_ptr<AccountImpl> account_;
    //std::shared_ptr<RootImpl> root_;

    friend class unity::storage::qt::Item;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
