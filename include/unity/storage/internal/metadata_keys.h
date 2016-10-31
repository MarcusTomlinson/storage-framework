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

enum class MetadataType { int64, iso_8601_date_time };

static std::unordered_map<std::string, MetadataType> const known_metadata =
{
    { metadata::SIZE_IN_BYTES, MetadataType::int64 },
    { metadata::CREATION_TIME, MetadataType::iso_8601_date_time },
    { metadata::LAST_MODIFIED_TIME, MetadataType::iso_8601_date_time }
};

}  // namespace metadata
}  // namespace storage
}  // namespace unity
