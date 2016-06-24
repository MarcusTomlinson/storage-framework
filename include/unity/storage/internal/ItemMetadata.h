#pragma once

#include <unity/storage/common.h>

#include <QMap>
#include <QVariant>

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
