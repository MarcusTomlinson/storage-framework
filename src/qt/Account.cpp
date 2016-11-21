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

#include <unity/storage/qt/Account.h>

#include <unity/storage/qt/internal/AccountImpl.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{

Account::Account()
    : p_(make_shared<internal::AccountImpl>())
{
}

Account::Account(shared_ptr<internal::AccountImpl> const& p)
    : p_(p)
{
    assert(p);
}

Account::Account(Account const& other)
    : p_(other.p_)
{
}

Account::Account(Account&& other)
    : p_(make_shared<internal::AccountImpl>())
{
    p_->is_valid_ = false;
    swap(p_, other.p_);
}

Account::~Account() = default;

Account& Account::operator=(Account const& other)
{
    if (this == &other)
    {
        return *this;
    }
    p_ = other.p_;
    return *this;
}

Account& Account::operator=(Account&& other)
{
    p_->is_valid_ = false;
    swap(p_, other.p_);
    return *this;
}

bool Account::isValid() const
{
    return p_->is_valid_;
}

QString Account::busName() const
{
    return p_->busName();
}

QString Account::objectPath() const
{
    return p_->objectPath();
}

QString Account::name() const
{
    return p_->name();
}

QString Account::providerName() const
{
    return p_->providerName();
}

QString Account::iconName() const
{
    return p_->iconName();
}

ItemListJob* Account::roots(QStringList const& keys) const
{
    return p_->roots(keys);
}

ItemJob* Account::get(QString const& itemId, QStringList const& keys) const
{
    return p_->get(itemId, keys);
}

bool Account::operator==(Account const& other) const
{
    return p_->operator==(*other.p_);
}

bool Account::operator!=(Account const& other) const
{
    return p_->operator!=(*other.p_);
}

bool Account::operator<(Account const& other) const
{
    return p_->operator<(*other.p_);
}

bool Account::operator<=(Account const& other) const
{
    return p_->operator<=(*other.p_);
}

bool Account::operator>(Account const& other) const
{
    return p_->operator>(*other.p_);
}

bool Account::operator>=(Account const& other) const
{
    return p_->operator>=(*other.p_);
}

size_t Account::hash() const
{
    return p_->hash();
}

// Due to potentially different size of size_t and uint, hash() and qhash() may not return the same value.

uint qHash(Account const& acc)
{
    return acc.hash();
}

}  // namespace qt
}  // namespace storage
}  // namespace unity
