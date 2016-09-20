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

#include <unity/storage/qt/ItemJob.h>

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

class ItemJobImpl : public QObject
{
    Q_OBJECT
public:
    virtual ~ItemJobImpl() = default;

    bool isValid() const;
    ItemJob::Status status() const;
    StorageError error() const;
    Item item() const;

    static ItemJob* make_item_job(std::shared_ptr<AccountImpl> const& account,
                                  QString const& method,
                                  QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                                  std::function<void(storage::internal::ItemMetadata const&)> const& validate);
    static ItemJob* make_item_job(StorageError const& e);

private:
    ItemJobImpl(std::shared_ptr<AccountImpl> const& account,
                QString const& method,
                QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                std::function<void(storage::internal::ItemMetadata const&)> const& validate);
    ItemJobImpl(StorageError const& e);

    ItemJob::Status emit_status_changed(ItemJob::Status new_status) const;

    ItemJob* public_instance_;

    ItemJob::Status status_;
    StorageError error_;
    QString method_;
    std::shared_ptr<AccountImpl> account_;
    std::function<void(storage::internal::ItemMetadata const&)> validate_;
    Item item_;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
