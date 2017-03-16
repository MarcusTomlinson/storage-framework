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

#include <unity/storage/provider/internal/AccountData.h>
#include <unity/storage/internal/InactivityTimer.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/DBusPeerCache.h>
#include <unity/storage/provider/internal/PendingJobs.h>

#include <QDebug>

using namespace std;
using unity::storage::internal::InactivityTimer;

namespace unity {
namespace storage {
namespace provider {
namespace internal {

AccountData::AccountData(shared_ptr<ProviderBase> const& provider,
                         shared_ptr<DBusPeerCache> const& dbus_peer,
                         shared_ptr<InactivityTimer> const& inactivity_timer,
                         QDBusConnection const& bus,
                         QObject* parent)
    : QObject(parent), provider_(provider), dbus_peer_(dbus_peer),
      inactivity_timer_(inactivity_timer), jobs_(new PendingJobs(bus))
{
}

AccountData::~AccountData() = default;

ProviderBase& AccountData::provider()
{
    return *provider_;
}

DBusPeerCache& AccountData::dbus_peer()
{
    return *dbus_peer_;
}

shared_ptr<InactivityTimer> AccountData::inactivity_timer()
{
    return inactivity_timer_;
}

PendingJobs& AccountData::jobs()
{
    return *jobs_;
}

}
}
}
}
