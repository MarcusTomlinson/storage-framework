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
#include <QStringList>

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

/**
Class th provides access to account details.

Note that this class is an immutable value type: if you retrieve the details of an account, and
those details change in Online Accounts later, the change is not reflected in the Account
instance you retrieved earlier. To get the updated details, you must retrieve the latest
list of accounts again by calling Runtime::accounts() and use the accounts returned
by the corresponding AccountsJob.
*/

class Q_DECL_EXPORT Account final
{
    Q_GADGET
    /**
    \see \link isValid() const isValid()\endlink
    */
    Q_PROPERTY(bool isValid READ isValid FINAL)

    /**
    \see \link busName() const busName()\endlink
    */
    Q_PROPERTY(QString busName READ busName FINAL)

    /**
    \see \link objectPath() const objectPath()\endlink
    */
    Q_PROPERTY(QString objectPath READ objectPath FINAL)

    /**
    \see \link displayName() const displayName()\endlink
    */
    Q_PROPERTY(QString displayName READ displayName FINAL)

    /**
    \see \link providerName() const providerName()\endlink
    */
    Q_PROPERTY(QString providerName READ providerName FINAL)

    /**
    \see \link iconName() const iconName()\endlink
    */
    Q_PROPERTY(QString iconName READ iconName FINAL)

public:
    /**
    \brief Constructs an account.

    A default-constructed Account returns <code>false</code> from isValid(), and the remaining accessors return
    the empty string.
    */
    Account();

    /**
    \brief Destroys an account.
    */
    ~Account();

    /** @name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    Account(Account const&);
    Account(Account&&);
    Account& operator=(Account const&);
    Account& operator=(Account&&);
    //@}

    /**
    \brief Checks whether this account was successfully constructed.
    \return Returns <code>true</code> if the account contains valid details; <code>false</code> otherwise.
    */
    bool isValid() const;

    /**
    \brief Returns the DBus name of the corresponding provider.
    \return The DBus name or the empty string if isValid() returns <code>false</code>.
    */
    QString busName() const;

    /**
    \brief Returns the DBus object path of the corresponding provider.
    \return The DBus object path or the empty string if isValid() returns <code>false</code>.
    */
    QString objectPath() const;

    /**
    \brief Returns the display name of the account.
    \return The display name (such as "user@domain.com") or the empty string if isValid() returns <code>false</code>.
    */
    QString displayName() const;

    /**
    \brief Returns the name of the provider for the account.
    \return The provider name (such as "Nextcloud") or the empty string if isValid() returns <code>false</code>.
    */
    QString providerName() const;

    /**
    \brief Returns the name of an icon file.
    \return The name of a file containing an icon image or the empty string if isValid() returns <code>false</code>.
    */
    QString iconName() const;

    /**
    \brief Retrieves the list of available roots.
    \param keys A list of metadata keys for metadata items that should be returned by the provider.
    If the list is empty, the provider returns a default set of metadata items.
    \return An ItemListJob that, once complete, provides access to the available roots.
    \note You <i>must</i> deallocate the returned job by calling <code>delete</code>.
    \see Item, \link metadata Metadata\endlink
    */
    Q_INVOKABLE unity::storage::qt::ItemListJob* roots(QStringList const& keys = QStringList()) const;

    /**
    \brief Retrieves an item by identity.
    \return An ItemJob that, once complete, provides access to the item's details.
    \note You <i>must</i> deallocate the returned job by calling <code>delete</code>.
    \see Item
    */
    Q_INVOKABLE unity::storage::qt::ItemJob* get(QString const& itemId, QStringList const& keys = QStringList()) const;

    /** @name Comparison operators and hashing
    */
    //{@
    /**
    \brief Compares Account instances for equality.

    Account instances are equal if both are invalid or both are valid and all their details
    (bus name, object path, display name, provider name, and icon name) are equal.
    \param other The account to compare this account with.
    \return If all details of the accounts match, <code>true</code> is returned; <code>false</code> otherwise.
    */
    bool operator==(Account const& other) const;
    bool operator!=(Account const&) const;
    bool operator<(Account const&) const;
    bool operator<=(Account const&) const;
    bool operator>(Account const&) const;
    bool operator>=(Account const&) const;

    /**
    \brief Returns a hash value.
    \note The hash value is <i>not</i> necessarily the same as the one returned by
    \link unity::storage::qt::qHash(Account const& acc) qHash()\endlink, but <i>is</i> the same as the one returned by
    \link std::hash<unity::storage::qt::Account> std::hash<Account>()\endlink.
    \return A hash value for use with unordered containers.
    */
    size_t hash() const;
    //@}

private:
    ///@cond
    Account(std::shared_ptr<internal::AccountImpl> const& p);

    std::shared_ptr<internal::AccountImpl> p_;

    friend class internal::AccountImpl;
    friend class internal::ItemImpl;
    ///@endcond
};

/**
\brief Returns a hash value.
\note The hash value is <i>not</i> necessarily the same as the one returned by
\link Account::hash()\endlink.
\return A hash value for use with unordered containers.
*/
uint Q_DECL_EXPORT qHash(Account const& acc);

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::Account)
Q_DECLARE_METATYPE(QList<unity::storage::qt::Account>)

namespace std
{

/**
\brief Function template specialization in namespace <code>std</code> to allow use of
\link unity::storage::qt::Account Account\endlink instances with unordered containers.
*/
template<> struct Q_DECL_EXPORT hash<unity::storage::qt::Account>
{
    /**
    \brief Returns a hash value.
    \note The hash value is <i>not</i> necessarily the same as the one returned by
    \link unity::storage::qt::qHash(const Account& acc) qHash()\endlink (but <i>is</i> the same as the value returned
    by \link unity::storage::qt::Account::hash() Account::hash()\endlink).
    \return A hash value for use with unordered containers.
    \see \link unity::storage::qt::Account::hash Account::hash()\endlink
    */
    std::size_t operator()(unity::storage::qt::Account const& a) const
    {
        return a.hash();
    }
};

}  // namespace std
