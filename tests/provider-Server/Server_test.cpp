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
using unity::storage::provider::Server;
using unity::storage::provider::internal::ServerImpl;
using unity::storage::provider::testing::TestServer;

namespace {

const auto SECOND_CONNECTION_NAME = QStringLiteral("second-bus-connection");

const char BUS_NAME[] = "org.example.TestProvider";
const char SERVICE_ID[] = "oauth2-service";
const QString PROVIDER_ERROR = unity::storage::internal::DBUS_ERROR_PREFIX;

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
    void SetUp() override
    {
        ProviderFixture::SetUp();
    }

    void TearDown() override
    {
        ProviderFixture::TearDown();
    }

    std::unique_ptr<ProviderClient> client_;
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
