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


#include "registryadaptor.h"
#include <unity/storage/registry/Registry.h>
#include <unity/storage/registry/internal/RegistryAdaptor.h>
#include <unity/storage/internal/AccountDetails.h>
#include <unity/storage/internal/InactivityTimer.h>
#include <utils/DBusEnvironment.h>
#include <utils/gtest_printer.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <glib.h>
#include <gtest/gtest.h>
#pragma GCC diagnostic pop
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QSignalSpy>

#include <memory>

using namespace std;
using namespace unity::storage;
using unity::storage::internal::InactivityTimer;

namespace
{
const auto SERVICE_CONNECTION_NAME = QStringLiteral("service-session-bus");
}

class RegistryTests : public ::testing::Test
{
public:
    void SetUp() override
    {
        dbus_.reset(new DBusEnvironment);
        dbus_->start_services();

        // Set up registry
        service_connection_.reset(new QDBusConnection(
            QDBusConnection::connectToBus(dbus_->busAddress(),
                                          SERVICE_CONNECTION_NAME)));

        registry_.reset(
            new registry::internal::RegistryAdaptor(*service_connection_,
                                make_shared<InactivityTimer>(0)));
        new ::RegistryAdaptor(registry_.get());
        ASSERT_TRUE(service_connection_->registerObject(
                        registry::OBJECT_PATH, registry_.get()));
    }

    void TearDown() override
    {
        registry_.reset();
        service_connection_.reset();
        QDBusConnection::disconnectFromBus(SERVICE_CONNECTION_NAME);
        dbus_.reset();
    }

    unique_ptr<DBusEnvironment> dbus_;
    unique_ptr<QDBusConnection> service_connection_;
    unique_ptr<registry::internal::RegistryAdaptor> registry_;
};

TEST_F(RegistryTests, list_accounts)
{
    auto message = QDBusMessage::createMethodCall(
        service_connection_->baseService(), registry::OBJECT_PATH,
        registry::INTERFACE, QStringLiteral("ListAccounts"));
    QDBusPendingReply<QList<internal::AccountDetails>> reply =
        dbus_->connection().asyncCall(message);

    {
        QDBusPendingCallWatcher watcher(reply);
        QSignalSpy spy(&watcher, &QDBusPendingCallWatcher::finished);
        ASSERT_TRUE(spy.wait());
    }
    ASSERT_TRUE(reply.isValid()) << reply.error().message().toStdString();
    auto accounts = reply.value();

    ASSERT_EQ(3, accounts.size());

    auto test_account = accounts[0];
    EXPECT_EQ("com.canonical.StorageFramework.Provider.ProviderTest", test_account.busName);
    EXPECT_EQ("/provider/42", test_account.objectPath.path());
    EXPECT_EQ(42u, test_account.id);
    EXPECT_EQ("storage-provider-test", test_account.serviceId);
    EXPECT_EQ("Fake test account", test_account.displayName);
    EXPECT_EQ("Test Provider", test_account.providerName);
    EXPECT_EQ("", test_account.iconName);

    auto mcloud_account = accounts[1];
    EXPECT_EQ("com.canonical.StorageFramework.Provider.McloudProvider", mcloud_account.busName);
    EXPECT_EQ("/provider/99", mcloud_account.objectPath.path());
    EXPECT_EQ(99u, mcloud_account.id);
    EXPECT_EQ("storage-provider-mcloud", mcloud_account.serviceId);
    EXPECT_EQ("Fake mcloud account", mcloud_account.displayName);
    EXPECT_EQ("mcloud", mcloud_account.providerName);
    EXPECT_EQ("", mcloud_account.iconName);

    auto local_account = accounts[2];
    EXPECT_EQ("com.canonical.StorageFramework.Provider.Local", local_account.busName);
    EXPECT_EQ("/provider/0", local_account.objectPath.path());
    EXPECT_EQ(0u, local_account.id);
    EXPECT_EQ("", local_account.serviceId);
    EXPECT_EQ(g_get_user_name(), local_account.displayName);
    EXPECT_EQ("Local Provider", local_account.providerName);
    EXPECT_EQ("", local_account.iconName);
}


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    qDBusRegisterMetaType<internal::AccountDetails>();
    qDBusRegisterMetaType<QList<internal::AccountDetails>>();

    ::testing::InitGoogleTest(&argc, argv);
    int rc = RUN_ALL_TESTS();

    // Process any pending events to avoid bogus leak reports from valgrind.
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents();

    return rc;
}
