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

#pragma once

#include <unity/storage/provider/Credentials.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <QObject>
#include <QDBusConnection>
#pragma GCC diagnostic pop

namespace unity
{
namespace storage
{
namespace internal
{

class InactivityTimer;

}
namespace provider
{

class ProviderBase;

namespace internal
{

class DBusPeerCache;
class PendingJobs;

class AccountData : public QObject
{
    Q_OBJECT
public:
    AccountData(std::shared_ptr<ProviderBase> const& provider,
                std::shared_ptr<DBusPeerCache> const& dbus_peer,
                std::shared_ptr<unity::storage::internal::InactivityTimer> const& inactivity_timer,
                QDBusConnection const& bus,
                QObject* parent=nullptr);
    virtual ~AccountData();

    virtual void authenticate(bool interactive, bool invalidate_cache=false) = 0;
    virtual bool has_credentials() = 0;
    virtual Credentials const& credentials() = 0;

    ProviderBase& provider();
    DBusPeerCache& dbus_peer();
    std::shared_ptr<unity::storage::internal::InactivityTimer> inactivity_timer();
    PendingJobs& jobs();

Q_SIGNALS:
    void authenticated();

private:
    std::shared_ptr<ProviderBase> const provider_;
    std::shared_ptr<DBusPeerCache> const dbus_peer_;
    std::shared_ptr<unity::storage::internal::InactivityTimer> const inactivity_timer_;
    std::unique_ptr<PendingJobs> const jobs_;

    Q_DISABLE_COPY(AccountData)
};

}
}
}
}
