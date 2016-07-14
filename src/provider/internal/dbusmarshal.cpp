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
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#include <unity/storage/provider/internal/dbusmarshal.h>
#include <unity/storage/provider/ProviderBase.h>

#include <cstdint>
#include <stdexcept>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{

QDBusArgument& operator<<(QDBusArgument& argument, Item const& item)
{
    argument.beginStructure();
    argument << QString::fromStdString(item.item_id);
    argument << QString::fromStdString(item.parent_id);
    argument << QString::fromStdString(item.title);
    argument << QString::fromStdString(item.etag);
    argument << static_cast<int32_t>(item.type);
    argument.beginMap(QVariant::String, qMetaTypeId<QDBusVariant>());
    for (auto const& pair : item.metadata)
    {
        argument.beginMapEntry();
        argument << QString::fromStdString(pair.first) << QDBusVariant(QString::fromStdString(pair.second));
        argument.endMapEntry();
    }
    argument.endMap();
    argument.endStructure();
    return argument;
}

QDBusArgument const& operator>>(QDBusArgument const&, Item&)
{
    throw std::runtime_error("Item decode not implemented");
}

QDBusArgument& operator<<(QDBusArgument& argument, ItemList const& items)
{
    argument.beginArray(qMetaTypeId<Item>());
    for (auto const& item : items)
    {
        argument << item;
    }
    argument.endArray();
    return argument;
}

QDBusArgument const& operator>>(QDBusArgument const&, ItemList&)
{
    throw std::runtime_error("std::vector<Item> decode not implemented");
}

}
}
}
