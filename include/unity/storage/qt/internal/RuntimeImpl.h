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

#include <OnlineAccounts/Manager>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDBusConnection>
#pragma GCC diagnostic pop

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
    AccountsJob* accounts() const;
    StorageError shutdown();

    std::shared_ptr<OnlineAccounts::Manager> accounts_manager() const;

    Account make_test_account(QString const& bus_name,
                              QString const& object_path);

    Account make_test_account(QString const& bus_name,
                              QString const& object_path,
                              QString const& owner_id,
                              QString const& owner,
                              QString const& description);

private:
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
