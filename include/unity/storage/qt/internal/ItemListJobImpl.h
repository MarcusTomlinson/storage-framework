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

#include <unity/storage/qt/ItemListJob.h>

#include <unity/storage/qt/Account.h>
#include <unity/storage/qt/internal/Handler.h>
#include <unity/storage/qt/StorageError.h>

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

class RuntimeImpl;

class ItemListJobImpl : public QObject
{
    Q_OBJECT
public:
    virtual ~ItemListJobImpl() = default;

    bool isValid() const;
    ItemListJob::Status status() const;
    StorageError error() const;

    static ItemListJob* make_item_list_job(std::shared_ptr<AccountImpl> const& account,
                                           QString const& method,
                                           QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                                           std::function<bool(storage::internal::ItemMetadata const&)> const& validate);
    static ItemListJob* make_item_list_job(StorageError const& error);

private:
    ItemListJobImpl(std::shared_ptr<AccountImpl> const& account,
                    QString const& method,
                    QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                    std::function<bool(storage::internal::ItemMetadata const&)> const& validate);
    ItemListJobImpl(StorageError const& error);

    ItemListJob::Status emit_status_changed(ItemListJob::Status new_status) const;
    void emit_items_ready(QList<unity::storage::qt::Item> const& items) const;

    ItemListJob* public_instance_;

    ItemListJob::Status status_;
    StorageError error_;
    QString method_;
    std::shared_ptr<AccountImpl> account_;
    std::function<bool(storage::internal::ItemMetadata const&)> validate_;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
