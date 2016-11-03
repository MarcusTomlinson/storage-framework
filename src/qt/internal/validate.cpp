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

#include <unity/storage/qt/internal/validate.h>

#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/internal/metadata_keys.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>

#include <QDateTime>
#include <QDebug>
#include <QString>

using namespace unity::storage::internal;
using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

namespace
{

// Check that actual type and value match the expect type and value for a particular metadata entry.

void validate_type_and_value(QString const& prefix,
                             QMapIterator<QString, QVariant> actual,
                             unordered_map<string, metadata::MetadataType>::const_iterator known)
{
    using namespace unity::storage::metadata;

    switch (known->second)
    {
        case MetadataType::iso_8601_date_time:
        {
            if (actual.value().type() != QVariant::String)
            {
                QString msg = prefix + actual.key() + ": expected value of type QString, but received value of type "
                              + actual.value().typeName();
                throw StorageErrorImpl::local_comms_error(msg);
            }
            QDateTime dt = QDateTime::fromString(actual.value().toString(), Qt::ISODate);
            if (!dt.isValid())
            {
                QString msg = prefix + actual.key() + ": value \"" + actual.value().toString()
                              + "\" does not parse as ISO-8601 date";
                throw StorageErrorImpl::local_comms_error(msg);
            }
            auto timespec = dt.timeSpec();
            if (timespec == Qt::LocalTime)
            {
                QString msg = prefix + actual.key() + ": value \"" + actual.value().toString()
                              + "\" lacks a time zone specification";
                throw StorageErrorImpl::local_comms_error(msg);
            }
            break;
        }
        case MetadataType::non_zero_pos_int64:
        {
            auto variant = actual.value();
            if (variant.type() != QVariant::LongLong)
            {
                QString msg = prefix + actual.key() + ": expected value of type qlonglong, but received value of type "
                              + variant.typeName();
                throw StorageErrorImpl::local_comms_error(msg);
            }
            qint64 val = variant.toLongLong();
            if (val < 0)
            {
                QString msg = prefix + actual.key() + ": expected value >= 0, but received " + QString::number(val);
                throw StorageErrorImpl::local_comms_error(msg);
            }
            break;
        }
        case MetadataType::string:
        case MetadataType::boolean:
        {
            break;
        }
        default:
        {
            abort();  // Impossible.  // LCOV_EXCL_LINE
        }
    }
}

}  // namespace

void validate(QString const& method, ItemMetadata const& md)
{
    using namespace unity::storage::metadata;

    QString prefix = method + ": received invalid metadata from provider: ";

    // Basic sanity checks for mandatory fields.
    if (md.item_id.isEmpty())
    {
        throw StorageErrorImpl::local_comms_error(prefix + "item_id cannot be empty");
    }
    if (md.type != ItemType::root)
    {
        if (md.parent_ids.isEmpty())
        {
            throw StorageErrorImpl::local_comms_error(prefix + "file or folder must have at least one parent ID");
        }
        for (int i = 0; i < md.parent_ids.size(); ++i)
        {
            if (md.parent_ids.at(i).isEmpty())
            {
                throw StorageErrorImpl::local_comms_error(prefix + "parent_id of file or folder cannot be empty");
            }
        }
    }
    if (md.type == ItemType::root && !md.parent_ids.isEmpty())
    {
        throw StorageErrorImpl::local_comms_error(prefix + "parent_ids of root must be empty");
    }
    if (md.type != ItemType::root)  // Dropbox does not support metadata for roots.
    {
        if (md.name.isEmpty())
        {
            throw StorageErrorImpl::local_comms_error(prefix + "name cannot be empty");
        }
    }
    if (md.type == ItemType::file && md.etag.isEmpty())  // WebDav doesn't do etag for folders.
    {
        throw StorageErrorImpl::local_comms_error(prefix + "etag of a file cannot be empty");
    }

    // Sanity check metadata to make sure only known metadata keys appear.
    QMapIterator<QString, QVariant> actual(md.metadata);
    while (actual.hasNext())
    {
        actual.next();
        auto known = known_metadata.find(actual.key().toStdString());
        if (known == known_metadata.end())
        {
            qWarning().noquote().nospace() << prefix << "unknown metadata key: \"" << actual.key() << "\"";
        }
        else
        {
            validate_type_and_value(prefix, actual, known);
        }
    }

    // Sanity check metadata to make sure that mandatory fields are present.
    if (md.type == ItemType::file)
    {
        if (!md.metadata.contains(metadata::SIZE_IN_BYTES) ||
            !md.metadata.contains(metadata::LAST_MODIFIED_TIME))
        {
            QString msg = prefix + "missing key \"" + metadata::SIZE_IN_BYTES + "\" "
                          "in metadata for \"" + md.item_id + "\"";
            throw StorageErrorImpl::local_comms_error(msg);
        }
    }
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
