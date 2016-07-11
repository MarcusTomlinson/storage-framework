#pragma once

#include <unity/storage/common.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QMap>
#include <QVariant>
#pragma GCC diagnostic pop

namespace unity
{
namespace storage
{
namespace internal
{

struct ItemMetadata
{
    QString item_id;
    QString parent_id;
    QString name;
    QString etag;
    ItemType type;
    QMap<QString, QVariant> metadata;
};

}  // namespace internal
}  // namespace storage
}  // namespace unity
