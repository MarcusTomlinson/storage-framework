#pragma once

#include <unity/storage/internal/ItemMetadata.h>

#include <QDBusArgument>
#include <QMetaType>
#include <QVariant>

namespace unity
{
namespace storage
{
namespace internal
{

struct ItemMetadata;
QDBusArgument& operator<<(QDBusArgument& argument, storage::internal::ItemMetadata const& metadata);
QDBusArgument const& operator>>(QDBusArgument const& argument, storage::internal::ItemMetadata& metadata);

QDBusArgument& operator<<(QDBusArgument& argument, QList<storage::internal::ItemMetadata> const& md_list);
QDBusArgument const& operator>>(QDBusArgument const& argument, QList<storage::internal::ItemMetadata>& md_list);

}  // namespace internal
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::internal::ItemMetadata)
Q_DECLARE_METATYPE(QList<unity::storage::internal::ItemMetadata>)
