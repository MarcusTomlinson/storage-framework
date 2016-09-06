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

#include <unity/storage/qt/Item.h>

#include <QString>

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class AccountImpl
{
public:
    AccountImpl();
    AccountImpl(QString const& bus_name,
                QString const& object_path,
                QString const& owner_id,
                QString const& owner,
                QString const& description);
    AccountImpl(AccountImpl const&) = default;
    AccountImpl(AccountImpl&&) = default;
    ~AccountImpl() = default;
    AccountImpl& operator=(AccountImpl const&) = default;
    AccountImpl& operator=(AccountImpl&&) = default;

    QString ownerId() const;
    QString owner() const;
    QString description() const;

    ItemListJob* roots() const;

    bool operator==(AccountImpl const&) const;
    bool operator!=(AccountImpl const&) const;
    bool operator<(AccountImpl const&) const;
    bool operator<=(AccountImpl const&) const;
    bool operator>(AccountImpl const&) const;
    bool operator>=(AccountImpl const&) const;

    size_t hash() const;

    static Account make_account(QString const& bus_name,
                                QString const& object_path,
                                QString const& owner_id,
                                QString const& owner,
                                QString const& description);

private:
    Account* public_instance_;

    bool is_valid_;
    QString bus_name_;
    QString object_path_;
    QString owner_id_;
    QString owner_;
    QString description_;

    friend class unity::storage::qt::Account;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
