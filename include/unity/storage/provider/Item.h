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

namespace unity
{
namespace storage
{
namespace provider
{

// Note: When growing the set of supported variant types, add new types
// to the *end* of the list, and update the marshaling code in dbusmarshal.cpp.
typedef boost::variant<std::string, int64_t> MetadataValue;

struct UNITY_STORAGE_EXPORT Item
{
    std::string item_id;
    std::vector<std::string> parent_ids;
    std::string name;
    std::string etag;
    unity::storage::ItemType type;
    std::map<std::string, MetadataValue> metadata;
};

typedef std::vector<Item> ItemList;

}
}
}
