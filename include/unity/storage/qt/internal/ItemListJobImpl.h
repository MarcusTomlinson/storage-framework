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
    ItemListJobImpl(ItemListJob* public_instance, std::shared_ptr<RuntimeImpl> const& runtime);
    ItemListJobImpl(ItemListJob* public_instance, StorageError const& error);
    ItemListJobImpl(ItemListJobImpl const&) = default;
    ItemListJobImpl(ItemListJobImpl&&) = delete;
    virtual ~ItemListJobImpl() = default;
    ItemListJobImpl& operator=(ItemListJobImpl const&) = default;
    ItemListJobImpl& operator=(ItemListJobImpl&&) = delete;

    bool isValid() const;
    ItemListJob::Status status() const;
    StorageError error() const;
    QList<Account> accounts() const;

    static ItemListJob* make_item_list_job(
                std::shared_ptr<AccountImpl> const& account,
                QString const& method,
                std::function<QVector<Item>(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const&)> const& f,
                QObject* parent);

private:
    std::shared_ptr<RuntimeImpl> get_runtime(QString const& method) const;
    void emit_status_changed() const;

    ItemListJob* const public_instance_;

    ItemListJob::Status status_;
    StorageError error_;
    std::weak_ptr<RuntimeImpl> const runtime_;

    friend class unity::storage::qt::ItemListJob;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
