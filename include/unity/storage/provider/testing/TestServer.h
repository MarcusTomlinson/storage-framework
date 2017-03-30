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

#include <unity/storage/visibility.h>

#include <memory>
#include <string>

//@cond
namespace OnlineAccounts
{
class Account;
}
class QDBusConnection;
//@endcond

namespace unity
{
namespace storage
{
namespace provider
{

class ProviderBase;

namespace internal
{
class TestServerImpl;
}

namespace testing
{

/**
\brief Helper class to enable testing of provider implementations.


TestServer is a simple helper class that allows you to test a provider implementation
on a separate DBus connection. The class requires access to
an <a href="https://help.ubuntu.com/stable/ubuntu-help/accounts.html">Online Accounts</a>
service. (You can find a
<a href="http://bazaar.launchpad.net/~unity-api-team/storage-framework/trunk/view/head:/tests/utils/fake-online-accounts-daemon.py">
mock implementation</a> of Online Accounts as part of the source code if you do not want to test with the installed Online Accounts
service.
*/

class UNITY_STORAGE_EXPORT TestServer
{
public:
    /**
    \brief Constructs a TestServer instance.
    \param provider The provider implementation to be tested.
    \param account The account for the provider.
    \param connection The DBus connection to connect the provider to.
    \param object_path The DBus object path for the provider interface.
    */
    TestServer(std::shared_ptr<ProviderBase> const& provider,
               OnlineAccounts::Account* account,
               QDBusConnection const& connection,
               std::string const& object_path);
    ~TestServer();

    /**
    \brief Returns the DBus connection.
    \return The value of the <code>connection</code> parameter that was passed to the constructor.
    */
    QDBusConnection const& connection() const;

    /**
    \brief Returns the object path.
    \return The value of the <code>object_path</code> parameter that was passed to the constructor.
    */
    std::string const& object_path() const;

private:
    std::unique_ptr<internal::TestServerImpl> p_;
};

}
}
}
}
