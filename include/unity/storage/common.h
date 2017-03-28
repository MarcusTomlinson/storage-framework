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
    ignore_conflict
};

namespace metadata
{

static char constexpr SIZE_IN_BYTES[] = "size_in_bytes";            // int64_t, >= 0
static char constexpr CREATION_TIME[] = "creation_time";            // String, ISO 8601 format
static char constexpr LAST_MODIFIED_TIME[] = "last_modified_time";  // String, ISO 8601 format
static char constexpr CHILD_COUNT[] = "child_count";                // int64_t, >= 0
static char constexpr DESCRIPTION[] = "description";                // String
static char constexpr DISPLAY_NAME[] = "display_name";              // String
static char constexpr FREE_SPACE_BYTES[] = "free_space_bytes";      // int64_t, >= 0
static char constexpr USED_SPACE_BYTES[] = "used_space_bytes";      // int64_t, >= 0
static char constexpr CONTENT_TYPE[] = "content_type";              // String
static char constexpr WRITABLE[] = "writable";                      // Bool
static char constexpr MD5[] = "md5";                                // String
static char constexpr DOWNLOAD_URL[] = "download_url";              // String

static char constexpr ALL[] = "__ALL__";

}  // namespace metadata
}  // namespace storage
}  // namespace unity
