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

#include <exception>

class QDBusPendingCallWatcher;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{

std::exception_ptr unmarshal_exception(QDBusPendingCallWatcher const& call);

}  // namespace remote_client
}  // namespace internal
}  // client
}  // qt
}  // storage
}  // unity
