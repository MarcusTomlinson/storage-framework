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

#include <unity/storage/qt/StorageError.h>

#include <OnlineAccounts/Manager>
#include <QDBusConnection>

namespace unity
{
namespace storage
{
namespace qt
{

class AccountsJob;
class Runtime;

namespace internal
{

class RuntimeImpl : public std::enable_shared_from_this<RuntimeImpl>
{
public:
    RuntimeImpl();
    RuntimeImpl(QDBusConnection const& bus);
    RuntimeImpl(RuntimeImpl const&) = delete;
    RuntimeImpl(RuntimeImpl&&) = delete;
    ~RuntimeImpl();
    RuntimeImpl& operator=(RuntimeImpl const&) = delete;
    RuntimeImpl& operator=(RuntimeImpl&&) = delete;

    bool isValid() const;
    StorageError error() const;
    QDBusConnection connection() const;
    StorageError shutdown();
    AccountsJob* accounts() const;

    std::shared_ptr<OnlineAccounts::Manager> accounts_manager() const;

private:
    Runtime* public_instance_;

    bool is_valid_;
    StorageError error_;
    QDBusConnection conn_;
    std::shared_ptr<OnlineAccounts::Manager> accounts_manager_;

    friend class unity::storage::qt::Runtime;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
