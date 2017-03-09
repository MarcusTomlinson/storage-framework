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
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#include <unity/storage/provider/internal/ServerImpl.h>
#include <unity/storage/internal/EnvVars.h>
#include <unity/storage/provider/Exceptions.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/MainLoopExecutor.h>
#include <unity/storage/provider/internal/OnlineAccountData.h>
#include <unity/storage/provider/internal/dbusmarshal.h>
#include "provideradaptor.h"

#include <QDebug>

#include <stdexcept>

using namespace std;
using unity::storage::internal::EnvVars;
using unity::storage::internal::InactivityTimer;

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

ServerImpl::ServerImpl(ServerBase* server, string const& bus_name, string const& account_service_id)
    : server_(server)
    , bus_name_(bus_name)
    , service_id_(account_service_id)
    , trace_message_handler_("storage_provider")
{
    qRegisterMetaType<std::exception_ptr>();
    qDBusRegisterMetaType<Item>();
    qDBusRegisterMetaType<std::vector<Item>>();
}

ServerImpl::~ServerImpl() = default;

void ServerImpl::init(int& argc, char **argv, QDBusConnection *bus)
{
    if (bus)
    {
        bus_.reset(new QDBusConnection(*bus));
    }
    else
    {
        // Only initialise QCoreApplication if we haven't been passed
        // in an existing bus connection.
        app_.reset(new QCoreApplication(argc, argv));
        bus_.reset(new QDBusConnection(QDBusConnection::sessionBus()));
    }
    int const timeout = EnvVars::provider_timeout_ms();
    inactivity_timer_ = make_shared<InactivityTimer>(timeout);
    connect(inactivity_timer_.get(), &InactivityTimer::timeout,
            this, &ServerImpl::on_timeout);

    dbus_peer_ = make_shared<DBusPeerCache>(*bus_);

#ifdef SF_SUPPORTS_EXECUTORS
    // Ensure the executor is instantiated in the main thread.
    MainLoopExecutor::instance();
#endif

    manager_.reset(new OnlineAccounts::Manager("", *bus_));
    connect(manager_.get(), &OnlineAccounts::Manager::ready,
            this, &ServerImpl::on_account_manager_ready);
    connect(manager_.get(), &OnlineAccounts::Manager::accountAvailable,
            this, &ServerImpl::on_account_available);
}

void ServerImpl::run()
{
    app_->exec();
}

void ServerImpl::add_account(OnlineAccounts::Account* account)
{
    // Ignore if we already have access to the account
    if (interfaces_.find(account->id()) != interfaces_.end())
    {
        return;
    }

    qDebug() << "Found account" << account->id() << "for service" << account->serviceId();
    auto account_data = make_shared<OnlineAccountData>(
        server_->make_provider(), dbus_peer_, inactivity_timer_,
        *bus_, account);
    unique_ptr<ProviderInterface> iface(
        new ProviderInterface(account_data));
    // this instance is managed by Qt's parent/child memory management
    new ProviderAdaptor(iface.get());

    bus_->registerObject(QStringLiteral("/provider/%1").arg(account->id()), iface.get());
    interfaces_.emplace(account->id(), std::move(iface));

    // watch for account disable signals.
    connect(account, &OnlineAccounts::Account::disabled,
            this, &ServerImpl::on_account_disabled);

    Q_EMIT accountAdded();
}

void ServerImpl::remove_account(OnlineAccounts::Account* account)
{
    // Ignore if we don't know about this account
    if (interfaces_.find(account->id()) == interfaces_.end())
    {
        return;
    }

    qDebug() << "Disabled account" << account->id() << "for service" << account->serviceId();
    bus_->unregisterObject(QStringLiteral("/provider/%1").arg(account->id()));
    interfaces_.erase(account->id());

    Q_EMIT accountRemoved();
}

void ServerImpl::on_account_manager_ready()
{
    for (const auto& account : manager_->availableAccounts(QString::fromStdString(service_id_)))
    {
        add_account(account);
    }

    if (!bus_->registerService(QString::fromStdString(bus_name_)))
    {
        string msg = string("Could not acquire bus name: ") + bus_name_ + ": " + bus_->lastError().message().toStdString();
        throw ResourceException(msg, int(bus_->lastError().type()));
    }
    // TODO: claim bus name
    qDebug() << "Bus unique name:" << bus_->baseService();
}

void ServerImpl::on_account_available(OnlineAccounts::Account* account)
{
    // Or if the service ID doesn't match
    if (account->serviceId() != QString::fromStdString(service_id_))
    {
        return;
    }
    add_account(account);
}

void ServerImpl::on_account_disabled()
{
    auto account = static_cast<OnlineAccounts::Account*>(sender());
    remove_account(account);
}

void ServerImpl::on_timeout()
{
    int const timeout = EnvVars::provider_timeout_ms();
    qInfo() << "Exiting after" << timeout << "ms of idle time";
    app_->quit();
}

}
}
}
}
