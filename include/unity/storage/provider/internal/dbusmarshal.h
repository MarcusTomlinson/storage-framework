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

namespace internal
{

QVariant marshal_item(Item const& item);
QVariant marshal_item_list(std::vector<Item> const& list);

}
}
}
}

Q_DECLARE_METATYPE(unity::storage::provider::Item)
