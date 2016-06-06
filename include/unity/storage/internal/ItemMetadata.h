#pragma once

#include <QMetaType>

namespace unity
{
namespace storage
{
namespace internal
{

struct ItemMetadata
{
};

}  // namespace internal
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::internal::ItemMetadata)
