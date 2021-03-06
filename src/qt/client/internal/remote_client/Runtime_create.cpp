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

#include <unity/storage/qt/client/Runtime.h>

#include <unity/storage/qt/client/internal/remote_client/RuntimeImpl.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

Runtime::SPtr Runtime::create(QDBusConnection const& bus)
{
    auto impl = new internal::remote_client::RuntimeImpl(bus);
    Runtime::SPtr runtime(new Runtime(impl));
    impl->set_public_instance(weak_ptr<Runtime>(runtime));
    return runtime;
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
