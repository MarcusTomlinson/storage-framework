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

#include <unity/storage/qt/AccountsJob.h>

#include <QTimer>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class AccountsJobImpl : public QObject
{
    Q_OBJECT
public:
    AccountsJobImpl(AccountsJob* public_instance, std::shared_ptr<RuntimeImpl> const& runtime);
    AccountsJobImpl(AccountsJob* public_instance, StorageError const& error);
    virtual ~AccountsJobImpl() = default;

    bool isValid() const;
    AccountsJob::Status status() const;
    StorageError error() const;
    QList<Account> accounts() const;

private Q_SLOTS:
    void manager_ready();
    void timeout();

private:
    std::shared_ptr<RuntimeImpl> get_runtime(QString const& method) const;
    void initialize_accounts();
    AccountsJob::Status emit_status_changed(AccountsJob::Status new_status) const;

    AccountsJob* const public_instance_;

    AccountsJob::Status status_;
    StorageError error_;
    QList<unity::storage::qt::Account> accounts_;
    std::weak_ptr<RuntimeImpl> const runtime_;
    QTimer timer_;

    friend class unity::storage::qt::AccountsJob;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
