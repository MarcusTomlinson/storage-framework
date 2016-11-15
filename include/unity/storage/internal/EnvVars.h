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

#include <unity/storage/registry/Registry.h>

#include <string>

namespace unity
{
namespace storage
{
namespace internal
{

constexpr char const* REGISTRY_IDLE_TIMEOUT = "SF_REGISTRY_IDLE_TIMEOUT";  // Seconds, 0 means "never"
constexpr int REGISTRY_IDLE_TIMEOUT_DFLT = 30;

constexpr char const* REGISTRY_OBJECT_PATH = "SF_REGISTRY_OBJECT_PATH";
constexpr char const* REGISTRY_OBJECT_PATH_DFLT = registry::OBJECT_PATH;

#if 0
constexpr char const* ONLINE_ACCOUNTS_OBJECT_PATH = "SF_ONLINE_ACCOUNTS_OBJECT_PATH";
constexpr char const* ONLINE_ACCOUNTS_OBJECT_PATH_DFLT = registry::OBJECT_PATH;
#endif

// Helper class to make retrieval of environment variables type-safe and
// to sanity check the setting, if applicable. Also returns a default
// setting, if applicable.

class EnvVars
{
public:
    static int registry_timeout_ms();
    static std::string registry_object_path();

    // Returns value of var_name in the environment, if set, and an empty string otherwise.
    // Can be used for any environment variable, not just the ones defined above.
    static std::string get(char const* var_name);
};

}  // namespace internal
}  // namespace storage
}  // namespace unity
