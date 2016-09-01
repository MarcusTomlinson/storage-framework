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

#include <unity/storage/qt/internal/RuntimeImpl>

#include <unity/storage/qt/internal/StorageErrorImpl>

#include <QDBusError>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

RuntimeImpl::RuntimeImpl()
    : is_valid_(true)
    , conn_(QDBusConnection::sessionBus())
{
    if (!conn_.isConnected())
    {
        is_valid_ = false;
        QString msg = "Runtime: cannot connect to session bus: " + conn_.lastError().message();
        last_error_ = StorageErrorImpl::local_comms_error(msg);
    }
}

RuntimeImpl::RuntimeImpl(QDBusConnection const& bus)
    : is_valid_(true)
    , conn_(bus)
{
    if (!conn_.isConnected())
    {
        is_valid_ = false;
        QString msg = "Runtime: DBus connection is not connected";
        last_error_ = StorageErrorImpl::local_comms_error(msg);
    }
}

RuntimeImpl::~RuntimeImpl()
{
    shutdown();
}

bool RuntimeImpl::isValid() const
{
    return is_valid_;
}

StorageError RuntimeImpl::lastError() const
{
    return last_error_;
}

ItemListJob* RuntimeImpl::accounts() const
{
    return nullptr;  // TODO
}

StorageError RuntimeImpl::shutdown()
{
    if (is_valid_)
    {
        is_valid_ = false;
    }
    return StorageError();
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
