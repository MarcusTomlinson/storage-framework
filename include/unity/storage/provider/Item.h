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
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#pragma once

#include <unity/storage/common.h>
#include <unity/storage/visibility.h>

#include <boost/variant.hpp>

#include <map>
#include <vector>

namespace unity
{
namespace storage
{
namespace provider
{

/**
\brief The type of a metadata item (key&ndash;value pair).

For boolean values, use <code>int64_t</code> with a zero or non-zero value to
indicate <code>false</code> or <code>true</code>.

Well-known metadata keys are defined in \ref common.h.
*/
// Note: When growing the set of supported variant types, add new types
// to the *end* of the list, and update the marshaling code in dbusmarshal.cpp.
typedef boost::variant<std::string, int64_t> MetadataValue;

/**
\brief Details about a storage item.
*/

struct UNITY_STORAGE_EXPORT Item
{
    /**
    \brief The unique identity of the item.

    See <a href="index.html#identity">File and Folder Identity</a> for detailed semantics.
    */
    std::string item_id;

    /**
    \brief The list of parent folder identities.

    \note Depending on the provider, it is possible for a file or folder to have more than one
    parent folder.
    */
    std::vector<std::string> parent_ids;

    /**
    \brief The name of a file or folder.

    \note A provider may create a file or folder with a name other than the name that was requested by the
    client (such as folding upper case to lower case). This field contains the name of the item as created
    by the provider, not the name requested by the client.
    */
    std::string name;

    /**
    \brief The ETag of a file.

    \note For folders, the ETag is may be empty because not all providers support ETags for folders.
    */
    std::string etag;

    /**
    \brief The item type (file, folder, or root,).
    */
    unity::storage::ItemType type;

    /**
    \brief Additional metadata for a file or folder.
    */
    std::map<std::string, MetadataValue> metadata;
};

typedef std::vector<Item> ItemList;

}
}
}
