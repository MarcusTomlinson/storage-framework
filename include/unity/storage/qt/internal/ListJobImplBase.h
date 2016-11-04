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
#include <unity/storage/qt/StorageError.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDBusPendingReply>
#pragma GCC diagnostic pop

namespace unity
{
namespace storage
{
namespace internal
{

class ItemMetadata;

}  // namespace internal

namespace qt
{
namespace internal
{

class AccountImpl;
class MultiItemJobImpl;

class ListJobImplBase : public QObject
{
public:
    ListJobImplBase();  // Makes job in Finished state.
    ListJobImplBase(std::shared_ptr<AccountImpl> const& account_impl,
                    QString const& method,
                    std::function<void(storage::internal::ItemMetadata const&)> const& validate);
    ListJobImplBase(StorageError const& error);
    virtual ~ListJobImplBase() = default;

    bool isValid() const;
    ItemListJob::Status status() const;
    StorageError error() const;

    void set_public_instance(ItemListJob* p);

    static ItemListJob* make_job(StorageError const& error);
    static ItemListJob* make_empty_job();

protected:
    ItemListJob* public_instance_;
    ItemListJob::Status status_;
    StorageError error_;
    QString method_;
    std::shared_ptr<AccountImpl> account_impl_;
    std::function<void(storage::internal::ItemMetadata const&)> validate_;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
