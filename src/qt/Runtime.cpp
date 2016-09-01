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

#include <unity/storage/qt/Runtime>
#include <unity/storage/qt/internal/RuntimeImpl>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{

Runtime::Runtime(QObject* parent)
    : QObject(parent)
    , p_(new internal::RuntimeImpl)
{
}

Runtime::Runtime(QDBusConnection const& bus, QObject* parent)
    : QObject(parent)
    , p_(new internal::RuntimeImpl(bus))
{
}

Runtime::~Runtime() = default;

AccountsJob* Runtime::accounts() const
{
    return nullptr;  // TODO
}

}  // namespace qt
}  // namespace storage
}  // namespace unity
