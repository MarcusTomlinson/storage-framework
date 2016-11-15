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

#include <unity/storage/qt/Runtime.h>

#include <unity/storage/qt/internal/RuntimeImpl.h>

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

bool Runtime::isValid() const
{
    return p_->isValid();
}

StorageError Runtime::error() const
{
    return p_->error();
}

QDBusConnection Runtime::connection() const
{
    return p_->connection();
}

StorageError Runtime::shutdown()
{
    return p_->shutdown();
}

AccountsJob* Runtime::accounts() const
{
    return p_->accounts();
}

Account Runtime::make_test_account(QString const& bus_name,
                                   QString const& object_path,
                                   qlonglong id,
                                   QString const& service_id,
                                   QString const& display_name) const
{
    return p_->make_test_account(bus_name, object_path, id, service_id, display_name);
}

}  // namespace qt
}  // namespace storage
}  // namespace unity
