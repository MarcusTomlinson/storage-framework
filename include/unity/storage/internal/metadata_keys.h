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

#pragma once

#include <unity/storage/common.h>

#include <unordered_map>

namespace unity
{
namespace storage
{
namespace metadata
{

enum class MetadataType { non_zero_pos_int64, iso_8601_date_time, string, boolean };

static std::unordered_map<std::string, MetadataType> const known_metadata =
{
    { metadata::SIZE_IN_BYTES, MetadataType::non_zero_pos_int64 },
    { metadata::CREATION_TIME, MetadataType::iso_8601_date_time },
    { metadata::LAST_MODIFIED_TIME, MetadataType::iso_8601_date_time },
    { metadata::CHILD_COUNT, MetadataType::non_zero_pos_int64 },
    { metadata::DESCRIPTION, MetadataType::string },
    { metadata::DISPLAY_NAME, MetadataType::string },
    { metadata::FREE_SPACE_BYTES, MetadataType::non_zero_pos_int64 },
    { metadata::USED_SPACE_BYTES, MetadataType::non_zero_pos_int64 },
    { metadata::CONTENT_TYPE, MetadataType::string },
    { metadata::WRITABLE, MetadataType::boolean },
    { metadata::MD5, MetadataType::string },
    { metadata::DOWNLOAD_URL, MetadataType::string }
};

}  // namespace metadata
}  // namespace storage
}  // namespace unity
