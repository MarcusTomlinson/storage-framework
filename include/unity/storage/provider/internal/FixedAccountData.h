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

#pragma once

#include <unity/storage/provider/internal/AccountData.h>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

class FixedAccountData : public AccountData
{
    Q_OBJECT
public:
    FixedAccountData(std::shared_ptr<ProviderBase> const& provider,
                      std::shared_ptr<DBusPeerCache> const& dbus_peer,
                      std::shared_ptr<unity::storage::internal::InactivityTimer> const& inactivity_timer,
                      QDBusConnection const& bus,
                      QObject* parent=nullptr);
    virtual ~FixedAccountData();

    void authenticate(bool interactive, bool invalidate_cache=false) override;
    bool has_credentials() override;
    Credentials const& credentials() override;

private:
    Credentials credentials_ = boost::blank();

    Q_DISABLE_COPY(FixedAccountData)
};

}
}
}
}
