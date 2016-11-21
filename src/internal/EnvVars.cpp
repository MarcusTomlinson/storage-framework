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

#include <unity/storage/internal/EnvVars.h>

#include <cassert>
#include <QDebug>

#include <stdlib.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace internal
{

int EnvVars::registry_timeout_ms()
{
    int const dflt_val = REGISTRY_IDLE_TIMEOUT_DFLT * 1000;

    auto const val = get(REGISTRY_IDLE_TIMEOUT);
    if (val.empty())
    {
        return dflt_val;
    }
    try
    {
        size_t pos;
        auto int_val = stoi(val, &pos);
        if (pos != val.size())
        {
            throw invalid_argument("unexpected trailing character(s)");
        }
        if (int_val < 0)
        {
            throw invalid_argument("value must be >= 0");
        }
        return int_val;
    }
    catch (std::exception const& e)
    {
        qWarning().noquote().nospace() << "Invalid setting of env var " << QString::fromStdString(REGISTRY_IDLE_TIMEOUT)
                                       << " (\"" << QString::fromStdString(val) << "\"): " << e.what();
        qWarning().nospace() << "Using default value of " << REGISTRY_IDLE_TIMEOUT_DFLT;
    }
    return dflt_val;
}

string EnvVars::registry_object_path()
{
    auto const val = get(REGISTRY_OBJECT_PATH);
    if (val.empty())
    {
        return REGISTRY_OBJECT_PATH_DFLT;
    }
    return val;
}

string EnvVars::get(char const* var_name)
{
    assert(var_name != nullptr);

    auto p = getenv(var_name);
    if (!p)
    {
        return string();
    }
    return string(p);
}

}  // namespace internal
}  // namespace storage
}  // namespace unity
