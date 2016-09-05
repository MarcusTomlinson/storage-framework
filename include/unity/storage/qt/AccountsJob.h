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
#include <unity/storage/qt/StorageError.h>

#include <QObject>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class AccountsJobImpl;
class RuntimeImpl;

}  // namespace internal

class Account;
class Runtime;
class StorageError;

class Q_DECL_EXPORT AccountsJob final : public QObject
{
    // TODO: Add FINAL to all property macros?
    Q_PROPERTY(bool READ isValid)
    Q_PROPERTY(unity::storage::qt::Account::Status READ status NOTIFY statusChanged)
    Q_PROPERTY(unity::storage::qt::StorageError READ Error)
    Q_PROPERTY(QList<unity::storage::qt::Account> READ accounts)

public:
    AccountsJob(QObject* = nullptr);
    virtual ~AccountsJob();

    enum Status { Loading, Finished, Error };
    Q_ENUM(Status)

    bool isValid() const;
    Status status() const;
    StorageError error() const;
    QList<Account> accounts() const;
    
Q_SIGNALS:
    void statusChanged(unity::storage::qt::AccountsJob::Status status) const;

private:
    AccountsJob(std::shared_ptr<internal::RuntimeImpl> const& runtime, QObject* parent);
    AccountsJob(StorageError const& error, QObject* parent);

    std::unique_ptr<internal::AccountsJobImpl> const p_;

    friend class internal::RuntimeImpl;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
