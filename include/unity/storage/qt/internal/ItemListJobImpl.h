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

#include <unity/storage/qt/internal/ListJobImplBase.h>
#include <unity/storage/qt/ItemListJob.h>

#include <QDBusPendingReply>

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
class ItemImpl;

class ItemListJobImpl : public ListJobImplBase
{
    Q_OBJECT
public:
    virtual ~ItemListJobImpl() = default;

    static ItemListJob* make_job(std::shared_ptr<AccountImpl> const& account_impl,
                                 QString const& method,
                                 QDBusPendingReply<QList<storage::internal::ItemMetadata>>& reply,
                                 std::function<void(storage::internal::ItemMetadata const&)> const& validate);
    static ItemListJob* make_job(std::shared_ptr<ItemImpl> const& item_impl,
                                 QString const& method,
                                 QDBusPendingReply<QList<storage::internal::ItemMetadata>>& reply,
                                 std::function<void(storage::internal::ItemMetadata const&)> const& validate);
    static ItemListJob* make_job(StorageError const& error);

private:
    ItemListJobImpl() = default;
    ItemListJobImpl(std::shared_ptr<AccountImpl> const& account_impl,
                    QString const& method,
                    QDBusPendingReply<QList<storage::internal::ItemMetadata>>& reply,
                    std::function<void(storage::internal::ItemMetadata const&)> const& validate);
    ItemListJobImpl(std::shared_ptr<ItemImpl> const& item_impl,
                    QString const& method,
                    QDBusPendingReply<QList<storage::internal::ItemMetadata>>& reply,
                    std::function<void(storage::internal::ItemMetadata const&)> const& validate);

    std::shared_ptr<ItemImpl> item_impl_;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
