#pragma once

#include <QDBusArgument>
#include <QVariant>
#include <vector>

namespace unity
{
namespace storage
{
namespace provider
{

struct Item;

QDBusArgument& operator<<(QDBusArgument& argument, Item const& item);
QDBusArgument const& operator>>(QDBusArgument const& argument, Item& item);

QDBusArgument& operator<<(QDBusArgument& argument, std::vector<Item> const& items);
QDBusArgument const& operator>>(QDBusArgument const& argument, std::vector<Item>& items);

}
}
}

Q_DECLARE_METATYPE(unity::storage::provider::Item)
