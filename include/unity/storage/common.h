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

/**
\brief Indicates the type of an item.
*/

enum class ItemType
{
    file,         /*!< The item represents a file. */
    folder,       /*!< The item represents a (non-root) folder. */
    root,         /*!< The item represents a root folder. */
    LAST_ENTRY__  /*!< End of enumeration marker. */
};

/**
\brief Determines the behavior in case of an ETag mismatch.
*/

enum class ConflictPolicy
{
    error_if_conflict,  /*!< Return an error if the ETag has changed. */
    ignore_conflict,    /*!< Ignore ETag mismatch and overwrite or replace the file. */
    overwrite = ignore_conflict, // TODO: remove this, it's here only for compatibility with v1 API
};

/**
\brief This namespace defines well-known metadata keys.

See \ref common.h for details.
*/

namespace metadata
{

// Symbolic constants for well-known metadata keys.

// Doxygen bug makes it impossible to document each constant.

static char constexpr SIZE_IN_BYTES[] = "size_in_bytes";            // int64_t >= 0
static char constexpr CREATION_TIME[] = "creation_time";            // string, ISO 8601 format
static char constexpr LAST_MODIFIED_TIME[] = "last_modified_time";  // string, ISO 8601 format
static char constexpr CHILD_COUNT[] = "child_count";                // int64_t >= 0
static char constexpr DESCRIPTION[] = "description";                // string
static char constexpr DISPLAY_NAME[] = "display_name";              // string
static char constexpr FREE_SPACE_BYTES[] = "free_space_bytes";      // int64_t >= 0
static char constexpr USED_SPACE_BYTES[] = "used_space_bytes";      // int64_t >= 0
static char constexpr CONTENT_TYPE[] = "content_type";              // string
static char constexpr WRITABLE[] = "writable";                      // int64_t, 0 or 1
static char constexpr MD5[] = "md5";                                // string
static char constexpr DOWNLOAD_URL[] = "download_url";              // string

// A single-element vector containing the key ALL indicates that the client would like all available
// metadata to be returned by the provider.

static char constexpr ALL[] = "__ALL__";

}  // namespace metadata
}  // namespace storage
}  // namespace unity
