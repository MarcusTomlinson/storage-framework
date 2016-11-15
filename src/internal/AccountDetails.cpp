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

#include <unity/storage/internal/AccountDetails.h>

using namespace unity::storage::internal;

namespace unity
{
namespace storage
{
namespace internal
{

bool operator==(AccountDetails const& lhs, AccountDetails const& rhs)
{
   return    lhs.id == rhs.id
          && lhs.serviceId == rhs.serviceId
          && lhs.displayName == rhs.displayName;
}

bool operator!=(AccountDetails const& lhs, AccountDetails const& rhs)
{
    return !(lhs == rhs);
}

bool operator<(AccountDetails const& lhs, AccountDetails const& rhs)
{
    if (lhs.id < rhs.id)
    {
        return true;
    }
    if (lhs.id > rhs.id)
    {
        return false;
    }
    if (lhs.serviceId < rhs.serviceId)
    {
        return true;
    }
    if (lhs.serviceId > rhs.serviceId)
    {
        return false;
    }
    return lhs.displayName < rhs.displayName;
}

bool operator<=(AccountDetails const& lhs, AccountDetails const& rhs)
{
    return lhs < rhs || lhs == rhs;
}

bool operator>(AccountDetails const& lhs, AccountDetails const& rhs)
{
    return !(lhs <= rhs);
}

bool operator>=(AccountDetails const& lhs, AccountDetails const& rhs)
{
    return !(lhs < rhs);
}

QDBusArgument& operator<<(QDBusArgument& argument, storage::internal::AccountDetails const& account)
{
    argument.beginStructure();
    argument << account.providerId;
    argument << account.objectPath;
    argument << account.id;
    argument << account.serviceId;
    argument << account.displayName;
    argument << account.providerName;
    argument << account.iconName;
    argument.endStructure();
    return argument;
}

QDBusArgument const& operator>>(QDBusArgument const& argument, storage::internal::AccountDetails& account)
{
    argument.beginStructure();
    argument >> account.providerId;
    argument >> account.objectPath;
    argument >> account.id;
    argument >> account.serviceId;
    argument >> account.displayName;
    argument >> account.providerName;
    argument >> account.iconName;
    argument.endStructure();
    return argument;
}

QDBusArgument& operator<<(QDBusArgument& argument, QList<storage::internal::AccountDetails> const& acc_list)
{
    argument.beginArray(qMetaTypeId<storage::internal::AccountDetails>());
    for (auto const& acc : acc_list)
    {
        argument << acc;
    }
    argument.endArray();
    return argument;
}

QDBusArgument const& operator>>(QDBusArgument const& argument, QList<storage::internal::AccountDetails>& acc_list)
{
    acc_list.clear();
    argument.beginArray();
    while (!argument.atEnd())
    {
        AccountDetails acc;
        argument >> acc;
        acc_list.append(acc);
    }
    argument.endArray();
    return argument;
}

}  // namespace internal
}  // namespace storage
}  // namespace unity
