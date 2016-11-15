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

#include <unity/storage/qt/internal/RuntimeImpl.h>

#include "RegistryInterface.h"
#include <unity/storage/internal/dbusmarshal.h>
#include <unity/storage/internal/EnvVars.h>
#include <unity/storage/qt/internal/AccountImpl.h>
#include <unity/storage/qt/internal/AccountsJobImpl.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>
#include <unity/storage/qt/ItemJob.h>
#include <unity/storage/qt/ItemListJob.h>
#include <unity/storage/qt/Runtime.h>
#include <unity/storage/qt/VoidJob.h>
#include <unity/storage/registry/Registry.h>

#include <QDBusError>
#include <QDBusMetaType>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{
namespace
{

void register_meta_types()
{
    qRegisterMetaType<unity::storage::qt::AccountsJob::Status>();
    qRegisterMetaType<unity::storage::qt::Account>();
    qRegisterMetaType<QList<unity::storage::qt::Account>>();
    qRegisterMetaType<unity::storage::qt::Item>();
    qRegisterMetaType<QList<unity::storage::qt::Item>>();
    qRegisterMetaType<unity::storage::qt::ItemJob::Status>();
    qRegisterMetaType<unity::storage::qt::ItemListJob::Status>();
    qRegisterMetaType<unity::storage::qt::VoidJob::Status>();

    qDBusRegisterMetaType<unity::storage::internal::ItemMetadata>();
    qDBusRegisterMetaType<QList<unity::storage::internal::ItemMetadata>>();

    qDBusRegisterMetaType<unity::storage::internal::AccountDetails>();
    qDBusRegisterMetaType<QList<unity::storage::internal::AccountDetails>>();
}

}

RuntimeImpl::RuntimeImpl()
    : RuntimeImpl(QDBusConnection::sessionBus())
{
}

RuntimeImpl::RuntimeImpl(QDBusConnection const& conn)
    : is_valid_(true)
    , conn_(conn)
    , registry_(new RegistryInterface(registry::BUS_NAME,
                                      QString::fromStdString(storage::internal::EnvVars::registry_object_path()),
                                      conn_))
{
    register_meta_types();
}

RuntimeImpl::~RuntimeImpl()
{
    shutdown();
}

bool RuntimeImpl::isValid() const
{
    return is_valid_;
}

StorageError RuntimeImpl::error() const
{
    return error_;
}

QDBusConnection RuntimeImpl::connection() const
{
    return conn_;
}

AccountsJob* RuntimeImpl::accounts() const
{
    QString const method = "Runtime::accounts()";

    if (!is_valid_)
    {
        QString msg = "Runtime::accounts(): Runtime was destroyed previously";
        return AccountsJobImpl::make_job(StorageErrorImpl::runtime_destroyed_error(msg));
    }

    auto reply = registry_->ListAccounts();
    auto This = const_pointer_cast<RuntimeImpl>(shared_from_this());
    return AccountsJobImpl::make_job(This, method, reply);
}

StorageError RuntimeImpl::shutdown()
{
    if (is_valid_)
    {
        is_valid_ = false;
        return StorageError();
    }
    error_ = StorageErrorImpl::runtime_destroyed_error("Runtime::shutdown(): Runtime was destroyed previously");
    return error_;
}

Account RuntimeImpl::make_test_account(QString const& bus_name,
                                       QString const& object_path,
                                       qlonglong id,
                                       QString const& service_id,
                                       QString const& display_name)
{
    storage::internal::AccountDetails ad{bus_name, object_path, id, service_id, display_name, "", ""};
    return AccountImpl::make_account(shared_from_this(), ad);
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
