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
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/storage/internal/ItemMetadata.h>

#include <unity/storage/provider/metadata_keys.h>

#include <QDateTime>
#include <QDebug>
#include <QString>

using namespace std;

namespace unity
{
namespace storage
{
namespace internal
{
namespace
{

// Check that actual type and value match the expect type and value for a particular metadata entry.

QString validate_type_and_value(QString const& prefix,
                                QMapIterator<QString, QVariant> actual,
                                unordered_map<string, provider::MetadataType>::const_iterator known)
{
    using namespace unity::storage::provider;

    switch (known->second)
    {
        case MetadataType::iso_8601_date_time:
        {
            if (actual.value().type() != QVariant::String)
            {
                return prefix + actual.key() + ": expected value of type String, but received value of type "
                       + actual.value().typeName();
            }
            QDateTime dt = QDateTime::fromString(actual.value().toString(), Qt::ISODate);
            if (!dt.isValid())
            {
                return prefix + actual.key() + ": value \"" + actual.value().toString()
                       + "\" does not parse as ISO-8601 date";
            }
            auto timespec = dt.timeSpec();
            if (timespec == Qt::LocalTime)
            {
                return prefix + actual.key() + ": value \"" + actual.value().toString()
                       + "\" lacks a time zone specification";
            }
            break;
        }
        case MetadataType::int64:
        {
            if (actual.value().type() != QVariant::LongLong)
            {
                return prefix + actual.key() + ": expected value of type LongLong, but received value of type "
                       + actual.value().typeName();
            }
            break;
        }
        default:
        {
            abort();  // Impossible.  // LCOV_EXCL_LINE
        }
    }
    return "";  // No error found.
}

}  // namespace

QString validate(QString const& method, ItemMetadata const& md)
{
    using namespace unity::storage::provider;

    QString prefix = method + ": received invalid metadata from provider implementation: ";

    // Basic sanity checks for mandatory fields.
    if (md.item_id.isEmpty())
    {
        return prefix + "item_id cannot be empty";
    }
    if (md.type != ItemType::root)
    {
        if (md.parent_ids.isEmpty())
        {
            return prefix + "file or folder must have at least one parent ID";
        }
        for (int i = 0; i < md.parent_ids.size(); ++i)
        {
            if (md.parent_ids.at(i).isEmpty())
            {
                return prefix + "parent_id of file or folder cannot be empty";
            }
        }
    }
    if (md.type == ItemType::root && !md.parent_ids.isEmpty())
    {
        return prefix + "metadata: parent_ids of root must be empty";
    }
    if (md.name.isEmpty())
    {
        return prefix + "name cannot be empty";
    }
    if (md.etag.isEmpty())
    {
        return prefix + "etag cannot be empty";
    }

    // Sanity check metadata to make sure only known metadata keys appear.
    QMapIterator<QString, QVariant> actual(md.metadata);
    while (actual.hasNext())
    {
        actual.next();
        auto known = known_metadata.find(actual.key().toStdString());
        if (known == known_metadata.end())
        {
            qWarning() << prefix << "unknown metadata key:" << actual.key();
        }
        else
        {
            QString error = validate_type_and_value(prefix, actual, known);
            if (!error.isEmpty())
            {
                return error;
            }
        }
    }

    // Sanity check metadata to make sure that mandatory fields are present.
    if (md.type == ItemType::file)
    {
        if (!md.metadata.contains(SIZE_IN_BYTES))
        {
            return prefix + "missing key " + SIZE_IN_BYTES + " in metadata for " + md.item_id;
        }
        if (!md.metadata.contains(LAST_MODIFIED_TIME))
        {
            return prefix + "missing key " + LAST_MODIFIED_TIME + " in metadata for " + md.item_id;
        }
    }

    return "";  // No errors found.
}

}  // namespace internal
}  // namespace storage
}  // namespace unity