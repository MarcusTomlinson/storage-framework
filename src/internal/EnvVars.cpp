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
    return get_timeout_ms(REGISTRY_IDLE_TIMEOUT, REGISTRY_IDLE_TIMEOUT_DFLT);
}

int EnvVars::provider_timeout_ms()
{
    return get_timeout_ms(PROVIDER_IDLE_TIMEOUT, PROVIDER_IDLE_TIMEOUT_DFLT);
}

int EnvVars::get_timeout_ms(char const* var_name, int dflt)
{
    int timeout = dflt;

    auto const val = get(var_name);
    if (!val.empty())
    {
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
            timeout = int_val;
        }
        catch (std::exception const& e)
        {
            qWarning().noquote().nospace()
                << "Invalid setting of env var " << var_name
                << " (\"" << QString::fromStdString(val) << "\"): " << e.what();
            qWarning().nospace() << "Using default value of " << dflt;
        }
    }
    return timeout * 1000;
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
