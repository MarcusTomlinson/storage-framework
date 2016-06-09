#include <unity/storage/qt/client/internal/remote_client/dbusmarshal.h>

#include <cstdint>
#include <stdexcept>

using namespace std;

namespace unity
{
namespace storage
{

namespace internal
{

QDBusArgument& operator<<(QDBusArgument& argument, storage::internal::ItemMetadata const& metadata)
{
    throw std::runtime_error("ItemMetdata encode not implemented");
    argument.beginStructure();
#if 0
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
#endif
    argument.endStructure();
    return argument;
}

QDBusArgument const& operator>>(QDBusArgument const& argument, storage::internal::ItemMetadata& metadata)
{
}

QDBusArgument& operator<<(QDBusArgument& argument, QList<storage::internal::ItemMetadata> const& md_list)
{
    throw std::runtime_error("QList<ItemMetdata> encode not implemented");
#if 0
    argument.beginArray(qMetaTypeId<Item>());
    for (auto const& item : items)
    {
        argument << item;
    }
    argument.endArray();
    return argument;
#endif
}

QDBusArgument const& operator>>(QDBusArgument const& argument, QList<storage::internal::ItemMetadata>& md_list)
{
}

}  // namespace internal

namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{

}  // remote_client
}  // internal
}  // client
}  // qt
}  // storage
}  // unity
