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

#include <unity/storage/internal/dbus_error.h>
#include <unity/storage/provider/Exceptions.h>
#include <unity/storage/provider/Server.h>
#include <unity/storage/provider/internal/ServerImpl.h>

#include "../provider-ProviderInterface/TestProvider.h"

#include <utils/ProviderFixture.h>
#include <utils/gtest_printer.h>

#include <gtest/gtest.h>
#include <OnlineAccounts/Account>
#include <OnlineAccounts/Manager>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QSignalSpy>
#include <QSocketNotifier>
#include <QTimer>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <exception>
#include <memory>
#include <mutex>
#include <stdexcept>

using namespace std;
using unity::storage::ItemType;
using unity::storage::provider::Context;
using unity::storage::provider::ItemList;
using unity::storage::provider::PasswordCredentials;
using unity::storage::provider::Server;
using unity::storage::provider::internal::ServerImpl;
using unity::storage::provider::testing::TestServer;

namespace {

const auto SECOND_CONNECTION_NAME = QStringLiteral("second-bus-connection");

const char BUS_NAME[] = "org.example.TestProvider";
const char SERVICE_ID[] = "oauth2-service";

const char OA_BUS_NAME[] = "com.ubuntu.OnlineAccounts.Manager";
const char OA_OBJECT_PATH[] = "/com/ubuntu/OnlineAccounts/Manager";
const char OA_TEST_IFACE[] = "com.canonical.StorageFramework.Testing";

const string file_contents =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
    "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut "
    "enim ad minim veniam, quis nostrud exercitation ullamco laboris "
    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor "
    "in reprehenderit in voluptate velit esse cillum dolore eu fugiat "
    "nulla pariatur. Excepteur sint occaecat cupidatat non proident, "
    "sunt in culpa qui officia deserunt mollit anim id est laborum.";
}

class ServerTest : public ProviderFixture
{
protected:
    void update_account(const char* account_data)
    {
        auto msg = QDBusMessage::createMethodCall(
            OA_BUS_NAME, OA_OBJECT_PATH, OA_TEST_IFACE, "UpdateAccount");
        msg << account_data;
        QDBusPendingReply<void> reply = connection().asyncCall(msg);
        wait_for(reply);
        if (!reply.isValid()) {
            throw runtime_error(reply.error().message().toStdString());
        }
    }

    void remove_account(uint32_t account_id, const char* service_id)
    {
        auto msg = QDBusMessage::createMethodCall(
            OA_BUS_NAME, OA_OBJECT_PATH, OA_TEST_IFACE, "RemoveAccount");
        msg << account_id << service_id;
        QDBusPendingReply<void> reply = connection().asyncCall(msg);
        wait_for(reply);
        if (!reply.isValid()) {
            throw runtime_error(reply.error().message().toStdString());
        }
    }
};

TEST_F(ServerTest, accounts_available_on_start)
{
    unique_ptr<Server<TestProvider>> server(
        new Server<TestProvider>(BUS_NAME, SERVICE_ID));
    unique_ptr<ServerImpl> impl(
        new ServerImpl(server.get(), BUS_NAME, SERVICE_ID));

    QSignalSpy added_spy(impl.get(), &ServerImpl::accountAdded);

    char *argv[1];
    int argc = 0;
    impl->init(argc, argv, service_connection_.get());
    if (added_spy.count() == 0)
    {
        added_spy.wait();
    }

    ProviderClient client(BUS_NAME, "/provider/2", connection());
    auto reply = client.Roots(QList<QString>());
    wait_for(reply);
    ASSERT_TRUE(reply.isValid()) << reply.error().message().toStdString();
}

TEST_F(ServerTest, add_account)
{
    unique_ptr<Server<TestProvider>> server(
        new Server<TestProvider>(BUS_NAME, SERVICE_ID));
    unique_ptr<ServerImpl> impl(
        new ServerImpl(server.get(), BUS_NAME, SERVICE_ID));

    QSignalSpy added_spy(impl.get(), &ServerImpl::accountAdded);

    char *argv[1];
    int argc = 0;
    impl->init(argc, argv, service_connection_.get());
    if (added_spy.count() == 0)
    {
        added_spy.wait();
    }

    // Add a second account
    added_spy.clear();
    update_account(R"(Account(20, 'new account', 'oauth2-service',
                              OAuth2('access_token', 0, [])))");
    if (added_spy.count() == 0)
    {
        added_spy.wait();
    }

    ProviderClient client(BUS_NAME, "/provider/20", connection());
    auto reply = client.Roots(QList<QString>());
    wait_for(reply);
    ASSERT_TRUE(reply.isValid()) << reply.error().message().toStdString();
}

TEST_F(ServerTest, remove_account)
{
    unique_ptr<Server<TestProvider>> server(
        new Server<TestProvider>(BUS_NAME, SERVICE_ID));
    unique_ptr<ServerImpl> impl(
        new ServerImpl(server.get(), BUS_NAME, SERVICE_ID));

    QSignalSpy added_spy(impl.get(), &ServerImpl::accountAdded);
    QSignalSpy removed_spy(impl.get(), &ServerImpl::accountRemoved);

    char *argv[1];
    int argc = 0;
    impl->init(argc, argv, service_connection_.get());
    if (added_spy.count() == 0)
    {
        added_spy.wait();
    }

    ProviderClient client(BUS_NAME, "/provider/2", connection());
    auto reply = client.Roots(QList<QString>());
    wait_for(reply);
    ASSERT_TRUE(reply.isValid()) << reply.error().message().toStdString();

    // Remove the account
    remove_account(2, SERVICE_ID);
    if (removed_spy.count() == 0)
    {
        removed_spy.wait();
    }

    // And note that new method calls are rejected
    reply = client.Roots(QList<QString>());
    wait_for(reply);
    ASSERT_FALSE(reply.isValid());
    EXPECT_EQ("No such object path '/provider/2'",
              reply.error().message().toStdString());
}

class DataChangeProvider : public TestProvider
{
    boost::future<ItemList> roots(vector<string> const& metadata_keys,
                                  Context const& ctx) override
    {
        Q_UNUSED(metadata_keys);
        auto host = boost::get<PasswordCredentials>(ctx.credentials).host;
        boost::promise<ItemList> p;
        p.set_value({
            {"root_id", {}, host, "etag", ItemType::root, {}}
        });
        return p.get_future();

    }
};

TEST_F(ServerTest, account_data_changed)
{
    unique_ptr<Server<DataChangeProvider>> server(
        new Server<DataChangeProvider>(BUS_NAME, "password-host-service"));
    unique_ptr<ServerImpl> impl(
        new ServerImpl(server.get(), BUS_NAME, "password-host-service"));

    QSignalSpy added_spy(impl.get(), &ServerImpl::accountAdded);

    char *argv[1];
    int argc = 0;
    impl->init(argc, argv, service_connection_.get());
    if (added_spy.count() == 0)
    {
        added_spy.wait();
    }

    ProviderClient client(BUS_NAME, "/provider/4", connection());
    auto reply = client.Roots(QList<QString>());
    wait_for(reply);
    ASSERT_TRUE(reply.isValid()) << reply.error().message().toStdString();
    ASSERT_EQ(1, reply.value().size());
    EXPECT_EQ("http://www.example.com/", reply.value()[0].name);

    update_account(R"(Account(4, 'description', 'password-host-service',
                      Password('joe', 'secret'),
                      {'host': 'http://new.example.com/'}))");
    reply = client.Roots(QList<QString>());
    wait_for(reply);
    ASSERT_TRUE(reply.isValid()) << reply.error().message().toStdString();
    ASSERT_EQ(1, reply.value().size());
    EXPECT_EQ("http://new.example.com/", reply.value()[0].name);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    qDBusRegisterMetaType<unity::storage::internal::ItemMetadata>();
    qDBusRegisterMetaType<QList<unity::storage::internal::ItemMetadata>>();
    ::testing::InitGoogleTest(&argc, argv);
    int rc = RUN_ALL_TESTS();

    // Process any pending events to avoid bogus leak reports from valgrind.
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents();

    return rc;
}
