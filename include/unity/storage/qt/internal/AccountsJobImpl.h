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

namespace unity
{
namespace storage
{
namespace qt
{

namespace internal
{

class AccountsJobImpl
{
public:
    AccountsJobImpl(std::shared_ptr<RuntimeImpl> const& runtime);
    AccountsJobImpl(StorageError const& error);
    AccountsJobImpl(AccountsJobImpl const&) = default;
    AccountsJobImpl(AccountsJobImpl&&) = delete;
    ~AccountsJobImpl() = default;
    AccountsJobImpl& operator=(AccountsJobImpl const&) = default;
    AccountsJobImpl& operator=(AccountsJobImpl&&) = delete;

    bool isValid() const;
    AccountsJob::Status status() const;
    StorageError error() const;
    QList<Account> accounts() const;

private:
    bool is_valid_;
    AccountsJob::Status status_;
    StorageError error_;
    QList<unity::storage::qt::Account> accounts_;
    std::weak_ptr<RuntimeImpl> const runtime_;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
