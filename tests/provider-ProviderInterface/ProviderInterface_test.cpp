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

#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/DBusPeerCache.h>
#include <unity/storage/provider/internal/AccountData.h>
#include <unity/storage/provider/internal/ProviderInterface.h>
#include <unity/storage/provider/internal/dbusmarshal.h>
// generated DBus service adaptor
#include "../../src/provider/provideradaptor.h"

#include "TestProvider.h"
#include "ProviderClient.h"

#include <utils/DBusEnvironment.h>

#include <gtest/gtest.h>
#include <OnlineAccounts/Account>
#include <OnlineAccounts/Manager>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QSignalSpy>

#include <unistd.h>
#include <sys/types.h>
#include <exception>
#include <memory>
#include <mutex>
#include <stdexcept>

using namespace std;
using unity::storage::ItemType;
using namespace unity::storage::provider;

namespace {

const auto SERVICE_CONNECTION_NAME = QStringLiteral("service-session-bus");
const auto BUS_PATH = QStringLiteral("/provider");
const auto PROVIDER_IFACE = QStringLiteral("com.canonical.StorageFramework.Provider");
const char PROVIDER_ERROR[] = "com.canonical.StorageFramework.Provider.Error";

}


class ProviderInterfaceTest : public ::testing::Test
{
public:
    QDBusConnection const& connection()
    {
        return dbus_->connection();
    }

    void make_provider(unique_ptr<ProviderBase>&& provider)
    {
        account_manager_->waitForReady();
        OnlineAccounts::Account* account = account_manager_->account(
            2, "oauth2-service");
        ASSERT_NE(nullptr, account);

        auto peer_cache = make_shared<internal::DBusPeerCache>(*service_connection_);
        auto account_data = make_shared<internal::AccountData>(
            move(provider), peer_cache, *service_connection_, account);
        provider_interface_.reset(new internal::ProviderInterface(account_data));
        new ProviderAdaptor(provider_interface_.get());
        service_connection_->registerObject(BUS_PATH, provider_interface_.get());

        client_.reset(new ProviderClient(service_connection_->baseService(),
                                         BUS_PATH,
                                         connection()));
    }

    void wait_for(QDBusPendingCall const& call) {
        QDBusPendingCallWatcher watcher(call);
        QSignalSpy spy(&watcher, &QDBusPendingCallWatcher::finished);
        ASSERT_TRUE(spy.wait());
    }

protected:
    void SetUp() override
    {
        dbus_.reset(new DBusEnvironment);
        dbus_->start_services();
        service_connection_.reset(
            new QDBusConnection(QDBusConnection::connectToBus(
                dbus_->busAddress(), SERVICE_CONNECTION_NAME)));
        account_manager_.reset(new OnlineAccounts::Manager(
                                   "", *service_connection_));
    }

    void TearDown() override
    {
        client_.reset();
        provider_interface_.reset();
        service_connection_.reset();
        QDBusConnection::disconnectFromBus(SERVICE_CONNECTION_NAME);
        dbus_.reset();
    }

    unique_ptr<DBusEnvironment> dbus_;
    unique_ptr<QDBusConnection> service_connection_;
    unique_ptr<OnlineAccounts::Manager> account_manager_;
    unique_ptr<internal::ProviderInterface> provider_interface_;
    unique_ptr<ProviderClient> client_;
};


TEST_F(ProviderInterfaceTest, roots)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->Roots();
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
    EXPECT_EQ(1, reply.value().size());
    auto root = reply.value()[0];
    EXPECT_EQ("root_id", root.item_id);
    EXPECT_EQ("", root.parent_id);
    EXPECT_EQ("Root", root.name);
    EXPECT_EQ("etag", root.etag);
    EXPECT_EQ(ItemType::root, root.type);
}

TEST_F(ProviderInterfaceTest, list)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->List("root_id", "");
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
    auto items = reply.argumentAt<0>();
    QString page_token = reply.argumentAt<1>();
    ASSERT_EQ(2, items.size());
    EXPECT_EQ("child1_id", items[0].item_id);
    EXPECT_EQ("child2_id", items[1].item_id);
    EXPECT_EQ("page_token", page_token);

    reply = client_->List("root_id", page_token);
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
    items = reply.argumentAt<0>();
    page_token = reply.argumentAt<1>();
    ASSERT_EQ(2, items.size());
    EXPECT_EQ("child3_id", items[0].item_id);
    EXPECT_EQ("child4_id", items[1].item_id);
    EXPECT_EQ("", page_token);

    // Try a bad page token
    reply = client_->List("root_id", "bad_page_token");
    wait_for(reply);
    EXPECT_TRUE(reply.isError());
    EXPECT_EQ(PROVIDER_ERROR, reply.error().name());
    EXPECT_EQ("Unknown page token", reply.error().message());

    reply = client_->List("no_such_folder_id", "");
    wait_for(reply);
    EXPECT_TRUE(reply.isError());
    EXPECT_EQ(PROVIDER_ERROR, reply.error().name());
    EXPECT_EQ("Unknown folder", reply.error().message());
}


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    qDBusRegisterMetaType<unity::storage::internal::ItemMetadata>();
    qDBusRegisterMetaType<QList<unity::storage::internal::ItemMetadata>>();
    qDBusRegisterMetaType<unity::storage::provider::Item>();
    qDBusRegisterMetaType<std::vector<unity::storage::provider::Item>>();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
