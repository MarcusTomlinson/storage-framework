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

#include <unity/storage/internal/AccountDetails.h>
#include <unity/storage/qt/AccountsJob.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDBusPendingReply>
#pragma GCC diagnostic pop

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class RuntimeImpl;

class AccountsJobImpl : public QObject
{
    Q_OBJECT
public:
    AccountsJobImpl(std::shared_ptr<RuntimeImpl> const& runtime_impl,
                    QString const& method,
                    QDBusPendingReply<QList<storage::internal::AccountDetails>>& reply);
    AccountsJobImpl(StorageError const& error);
    virtual ~AccountsJobImpl() = default;

    bool isValid() const;
    AccountsJob::Status status() const;
    StorageError error() const;
    QList<Account> accounts() const;
    QVariantList accountsAsVariantList() const;

    static AccountsJob* make_job(std::shared_ptr<RuntimeImpl> const& runtime_impl,
                                 QString const& method,
                                 QDBusPendingReply<QList<storage::internal::AccountDetails>>& reply);
    static AccountsJob* make_job(StorageError const& e);

private:
    std::shared_ptr<RuntimeImpl> get_runtime_impl(QString const& method) const;
    AccountsJob::Status emit_status_changed(AccountsJob::Status new_status) const;

    AccountsJob* public_instance_;
    AccountsJob::Status status_;
    StorageError error_;
    std::weak_ptr<RuntimeImpl> const runtime_impl_;
    QList<Account> accounts_;

    friend class unity::storage::qt::AccountsJob;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
