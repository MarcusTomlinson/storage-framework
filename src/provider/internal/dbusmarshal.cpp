#include <unity/storage/provider/internal/dbusmarshal.h>
#include <unity/storage/provider/ProviderBase.h>

#include <cstdint>

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
    argument.beginMap(QVariant::Int, qMetaTypeId<QDBusVariant>());
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

namespace internal
{

QVariant marshal_item(Item const& item)
{
    QDBusArgument argument;
    argument << item;
    return argument.asVariant();
}

QVariant marshal_item_list(std::vector<Item> const& list)
{
    QDBusArgument argument;
    argument.beginArray(qMetaTypeId<Item>());
    for (auto const& item : list)
    {
        argument << item;
    }
    argument.endArray();
    return argument.asVariant();
}

}
}
}
}
