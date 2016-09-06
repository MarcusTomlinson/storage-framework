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

#include <unity/storage/qt/internal/AccountImpl.h>

#include <unity/storage/qt/Account.h>

#include <boost/functional/hash.hpp>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

AccountImpl::AccountImpl()
    : is_valid_(false)
{
}

AccountImpl::AccountImpl(QString const& bus_name,
                         QString const& object_path,
                         QString const& owner_id,
                         QString const& owner,
                         QString const& description)
    : bus_name_(bus_name)
    , object_path_(object_path)
    , owner_id_(owner_id)
    , owner_(owner)
    , description_(description)
{
    assert(!bus_name.isEmpty());
    assert(!object_path.isEmpty());
}

QString AccountImpl::owner() const
{
    return is_valid_ ? owner_ : "";
}

QString AccountImpl::ownerId() const
{
    return is_valid_ ? owner_id_ : "";
}

QString AccountImpl::description() const
{
    return is_valid_ ? description_ : "";
}

ItemListJob* AccountImpl::roots() const
{
    return nullptr;  // TODO
}

bool AccountImpl::operator==(AccountImpl const& other) const
{
    if (is_valid_)
    {
        return    other.is_valid_
               && owner_ == other.owner_
               && owner_id_ == other.owner_id_
               && description_ == other.description_;
    }
    return !other.is_valid_;
}

bool AccountImpl::operator!=(AccountImpl const& other) const
{
    return !operator==(other);
}

bool AccountImpl::operator<(AccountImpl const& other) const
{
    if (!is_valid_)
    {
        return other.is_valid_;
    }
    if (is_valid_ && !other.is_valid_)
    {
        return false;
    }
    assert(is_valid_ && other.is_valid_);
    if (owner_id_ < other.owner_id_)
    {
        return true;
    }
    if (owner_ < other.owner_)
    {
        return true;
    }
    if (description_ < other.description_)
    {
        return true;
    }
    return false;
}

bool AccountImpl::operator<=(AccountImpl const& other) const
{
    return operator<(other) || operator==(other);
}

bool AccountImpl::operator>(AccountImpl const& other) const
{
    return !operator<=(other);
}

bool AccountImpl::operator>=(AccountImpl const& other) const
{
    return !operator<(other);
}

size_t AccountImpl::hash() const
{
    if (!is_valid_)
    {
        return 0;
    }
    size_t hash = 0;
    boost::hash_combine(hash, owner_.toStdString());
    boost::hash_combine(hash, owner_id_.toStdString());
    boost::hash_combine(hash, description_.toStdString());
    return hash;
}

Account AccountImpl::make_account(QString const& bus_name,
                                  QString const& object_path,
                                  QString const& owner_id,
                                  QString const& owner,
                                  QString const& description)
{
    unique_ptr<AccountImpl> p(new AccountImpl(bus_name, object_path, owner_id, owner, description));
    return Account(move(p));
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
