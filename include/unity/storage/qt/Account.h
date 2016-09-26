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

#include <QMetaType>

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class AccountImpl;
class ItemImpl;

}

class ItemJob;
class ItemListJob;

class Q_DECL_EXPORT Account final
{
    Q_GADGET
    Q_PROPERTY(bool READ isValid FINAL)
    Q_PROPERTY(QString READ owner FINAL)
    Q_PROPERTY(QString READ ownerId FINAL)
    Q_PROPERTY(QString READ description FINAL)

public:
    Account();
    Account(Account const&);
    Account(Account&&);
    ~Account();
    Account& operator=(Account const&);
    Account& operator=(Account&&);

    bool isValid() const;
    QString owner() const;
    QString ownerId() const;
    QString description() const;

    Q_INVOKABLE ItemListJob* roots() const;
    Q_INVOKABLE ItemJob* get(QString const& itemId) const;

    bool operator==(Account const&) const;
    bool operator!=(Account const&) const;
    bool operator<(Account const&) const;
    bool operator<=(Account const&) const;
    bool operator>(Account const&) const;
    bool operator>=(Account const&) const;

    size_t hash() const;

private:
    Account(std::shared_ptr<internal::AccountImpl> const& p);

    std::shared_ptr<internal::AccountImpl> p_;

    friend class internal::AccountImpl;
    friend class internal::ItemImpl;
};

// Note: qHash(Account) does *not* return the same hash value is std::hash<Account> because
//       std:hash() returns size_t (typically 64 bits), but qHash() returns uint (typically 32 bits).
uint Q_DECL_EXPORT qHash(Account const& acc);

}  // namespace qt
}  // namespace storage
}  // namespace unity

namespace std
{

template<> struct Q_DECL_EXPORT hash<unity::storage::qt::Account>
{
    std::size_t operator()(unity::storage::qt::Account const& a)
    {
        return a.hash();
    }
};

}  // namespace std
