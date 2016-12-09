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

#include <unity/storage/registry/internal/qdbus-last-error-msg.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDBusError>
#pragma GCC diagnostic pop

namespace unity
{
namespace storage
{
namespace registry
{
namespace internal
{

QString last_error_msg(QDBusConnection const& conn)
{
    auto msg = conn.lastError().message();
    if (!msg.isEmpty())
    {
        msg = ": " + msg;
    }
    return msg;
}

}  // namespace internal
}  // namespace registry
}  // namespace storage
}  // namespace unity
