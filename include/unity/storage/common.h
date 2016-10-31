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

namespace unity
{
namespace storage
{

enum class ItemType
{
    file,
    folder,
    root,
    LAST_ENTRY__
};

enum class ConflictPolicy
{
    error_if_conflict,
    overwrite,
};

namespace metadata
{

static char constexpr SIZE_IN_BYTES[] = "size_in_bytes";            // int64_t, >= 0
static char constexpr CREATION_TIME[] = "creation_time";            // String, ISO 8601 format
static char constexpr LAST_MODIFIED_TIME[] = "last_modified_time";  // String, ISO 8601 format

static char constexpr ALL[] = "__ALL__";

}  // namespace metadata

}  // namespace storage
}  // namespace unity
