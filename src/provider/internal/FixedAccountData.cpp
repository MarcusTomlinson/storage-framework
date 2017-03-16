/*
 * Copyright (C) 2017 Canonical Ltd
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

#include <unity/storage/provider/internal/FixedAccountData.h>

using namespace std;
using unity::storage::internal::InactivityTimer;

namespace unity {
namespace storage {
namespace provider {
namespace internal {

FixedAccountData::FixedAccountData(shared_ptr<ProviderBase> const& provider,
                                     shared_ptr<DBusPeerCache> const& dbus_peer,
                                     shared_ptr<InactivityTimer> const& inactivity_timer,
                                     QDBusConnection const& bus,
                                     QObject* parent)
    : AccountData(provider, dbus_peer, inactivity_timer, bus, parent)
{
}

FixedAccountData::~FixedAccountData() = default;

void FixedAccountData::authenticate(bool interactive, bool invalidate_cache)
{
    Q_UNUSED(interactive);
    Q_UNUSED(invalidate_cache);

    // Queue an emission of the authenticated signal.
    QMetaObject::invokeMethod(this, "authenticated", Qt::QueuedConnection);
}

bool FixedAccountData::has_credentials()
{
    return true;
}

Credentials const& FixedAccountData::credentials()
{
    return credentials_;
}

}
}
}
}
