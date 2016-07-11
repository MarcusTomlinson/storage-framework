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
    argument << QString::fromStdString(item.name);
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

QDBusArgument const& operator>>(QDBusArgument const& argument, Item& item)
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

QDBusArgument const& operator>>(QDBusArgument const& argument, ItemList& items)
{
    throw std::runtime_error("std::vector<Item> decode not implemented");
}

}
}
}
