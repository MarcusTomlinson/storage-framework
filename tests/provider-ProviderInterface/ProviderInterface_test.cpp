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

#include <unity/storage/internal/dbus_error.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/testing/TestServer.h>

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
#include <QSocketNotifier>

#include <unistd.h>
#include <sys/types.h>
#include <exception>
#include <memory>
#include <mutex>
#include <stdexcept>

using namespace std;
using unity::storage::ItemType;
using unity::storage::provider::ProviderBase;
using unity::storage::provider::testing::TestServer;

namespace {

const auto SERVICE_CONNECTION_NAME = QStringLiteral("service-session-bus");
const auto BUS_PATH = QStringLiteral("/provider");
const auto PROVIDER_IFACE = QStringLiteral("com.canonical.StorageFramework.Provider");
const QString PROVIDER_ERROR = unity::storage::internal::DBUS_ERROR_PREFIX;

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

        test_server_.reset(
            new TestServer(move(provider), account,
                           *service_connection_, BUS_PATH.toStdString()));

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
        test_server_.reset();
        service_connection_.reset();
        QDBusConnection::disconnectFromBus(SERVICE_CONNECTION_NAME);
        dbus_.reset();
    }

    unique_ptr<DBusEnvironment> dbus_;
    unique_ptr<QDBusConnection> service_connection_;
    unique_ptr<OnlineAccounts::Manager> account_manager_;
    unique_ptr<TestServer> test_server_;
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
    EXPECT_EQ(QVector<QString>(), root.parent_ids);
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
    EXPECT_EQ(PROVIDER_ERROR + "LogicException", reply.error().name()) << reply.error().name().toStdString();
    EXPECT_EQ("Unknown page token", reply.error().message()) << reply.error().message().toStdString();

    reply = client_->List("no_such_folder_id", "");
    wait_for(reply);
    EXPECT_TRUE(reply.isError());
    EXPECT_EQ(PROVIDER_ERROR + "NotExistsException", reply.error().name());
    EXPECT_EQ("Unknown folder", reply.error().message());
}

TEST_F(ProviderInterfaceTest, lookup)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->Lookup("root_id", "Filename");
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
    auto items = reply.value();
    ASSERT_EQ(1, items.size());
    auto item = items[0];
    EXPECT_EQ("child_id", item.item_id);
    EXPECT_EQ(QVector<QString>{ "root_id"}, item.parent_ids);
    EXPECT_EQ("Filename", item.name);
    EXPECT_EQ(ItemType::file, item.type);
}

TEST_F(ProviderInterfaceTest, metadata)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->Metadata("root_id");
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
    auto item = reply.value();
    EXPECT_EQ("root_id", item.item_id);
    EXPECT_EQ(QVector<QString>(), item.parent_ids);
    EXPECT_EQ("Root", item.name);
    EXPECT_EQ(ItemType::root, item.type);
}

TEST_F(ProviderInterfaceTest, create_folder)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->CreateFolder("root_id", "New Folder");
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
    auto item = reply.value();
    EXPECT_EQ("new_folder_id", item.item_id);
    EXPECT_EQ(QVector<QString>{ "root_id" }, item.parent_ids);
    EXPECT_EQ("New Folder", item.name);
    EXPECT_EQ(ItemType::folder, item.type);
}

TEST_F(ProviderInterfaceTest, create_file)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    const std::string file_contents = "Hello world!";
    QString upload_id;
    QDBusUnixFileDescriptor socket;
    {
        auto reply = client_->CreateFile("parent_id", "file name", file_contents.size(), "text/plain", false);
        wait_for(reply);
        ASSERT_TRUE(reply.isValid());
        upload_id = reply.argumentAt<0>();
        socket = reply.argumentAt<1>();
    }

    auto app = QCoreApplication::instance();
    QSocketNotifier notifier(socket.fileDescriptor(), QSocketNotifier::Write);
    size_t total_written = 0;
    QObject::connect(
        &notifier, &QSocketNotifier::activated,
        [&file_contents, app, &notifier, &total_written](int fd) {
            ssize_t n_written = write(fd, file_contents.data() + total_written, file_contents.size() - total_written);
            if (n_written < 0)
            {
                // Error writing
                notifier.setEnabled(false);
                app->quit();
            }
            total_written += n_written;
            if (total_written == file_contents.size())
            {
                notifier.setEnabled(false);
                app->quit();
            }
        });
    notifier.setEnabled(true);
    app->exec();
    socket.setFileDescriptor(-1);

    auto reply = client_->FinishUpload(upload_id);
    wait_for(reply);
    ASSERT_TRUE(reply.isValid()) << reply.error().message().toStdString();
    auto item = reply.value();
    EXPECT_EQ("new_file_id", item.item_id);
    EXPECT_EQ(QVector<QString>{ "parent_id" }, item.parent_ids);
    EXPECT_EQ("file name", item.name);
}

TEST_F(ProviderInterfaceTest, update)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    const std::string file_contents = "Hello world!";
    QString upload_id;
    QDBusUnixFileDescriptor socket;
    {
        auto reply = client_->Update("item_id", file_contents.size(), "old_etag");
        wait_for(reply);
        ASSERT_TRUE(reply.isValid());
        upload_id = reply.argumentAt<0>();
        socket = reply.argumentAt<1>();
    }

    auto app = QCoreApplication::instance();
    QSocketNotifier notifier(socket.fileDescriptor(), QSocketNotifier::Write);
    size_t total_written = 0;
    QObject::connect(
        &notifier, &QSocketNotifier::activated,
        [&file_contents, app, &notifier, &total_written](int fd) {
            ssize_t n_written = write(fd, file_contents.data() + total_written, file_contents.size() - total_written);
            if (n_written < 0)
            {
                // Error writing
                notifier.setEnabled(false);
                app->quit();
            }
            total_written += n_written;
            if (total_written == file_contents.size())
            {
                notifier.setEnabled(false);
                app->quit();
            }
        });
    notifier.setEnabled(true);
    app->exec();
    socket.setFileDescriptor(-1);

    auto reply = client_->FinishUpload(upload_id);
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
    auto item = reply.value();
    EXPECT_EQ("item_id", item.item_id);
}

TEST_F(ProviderInterfaceTest, upload_short_write)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    QString upload_id;
    QDBusUnixFileDescriptor socket;
    {
        auto reply = client_->Update("item_id", 100, "old_etag");
        wait_for(reply);
        ASSERT_TRUE(reply.isValid());
        upload_id = reply.argumentAt<0>();
        socket = reply.argumentAt<1>();
    }
    auto reply = client_->FinishUpload(upload_id);
    wait_for(reply);
    ASSERT_TRUE(reply.isError());
    EXPECT_EQ(PROVIDER_ERROR + "LogicException", reply.error().name());
    EXPECT_EQ("wrong number of bytes written", reply.error().message());
}

TEST_F(ProviderInterfaceTest, upload_long_write)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    const std::string file_contents = "Hello world!";
    QString upload_id;
    QDBusUnixFileDescriptor socket;
    {
        auto reply = client_->Update("item_id", file_contents.size() - 5, "old_etag");
        wait_for(reply);
        ASSERT_TRUE(reply.isValid());
        upload_id = reply.argumentAt<0>();
        socket = reply.argumentAt<1>();
    }

    auto app = QCoreApplication::instance();
    QSocketNotifier notifier(socket.fileDescriptor(), QSocketNotifier::Write);
    size_t total_written = 0;
    QObject::connect(
        &notifier, &QSocketNotifier::activated,
        [&file_contents, app, &notifier, &total_written](int fd) {
            ssize_t n_written = write(fd, file_contents.data() + total_written, file_contents.size() - total_written);
            if (n_written < 0)
            {
                // Error writing
                notifier.setEnabled(false);
                app->quit();
            }
            total_written += n_written;
            if (total_written == file_contents.size())
            {
                notifier.setEnabled(false);
                app->quit();
            }
        });
    notifier.setEnabled(true);
    app->exec();
    socket.setFileDescriptor(-1);

    auto reply = client_->FinishUpload(upload_id);
    wait_for(reply);
    ASSERT_TRUE(reply.isError());
    EXPECT_EQ("too many bytes written", reply.error().message().toStdString());
}

TEST_F(ProviderInterfaceTest, cancel_upload)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    QString upload_id;
    QDBusUnixFileDescriptor socket;
    {
        auto reply = client_->Update("item_id", 100, "old_etag");
        wait_for(reply);
        ASSERT_TRUE(reply.isValid());
        upload_id = reply.argumentAt<0>();
        socket = reply.argumentAt<1>();
    }
    auto reply = client_->CancelUpload(upload_id);
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
}

TEST_F(ProviderInterfaceTest, finish_upload_unknown)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->FinishUpload("no-such-upload");
    wait_for(reply);
    ASSERT_TRUE(reply.isError());
    EXPECT_EQ(PROVIDER_ERROR + "UnknownException", reply.error().name());
    EXPECT_EQ("unknown exception thrown by provider: map::at", reply.error().message());
}

TEST_F(ProviderInterfaceTest, download)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    QString download_id;
    QDBusUnixFileDescriptor socket;
    {
        auto reply = client_->Download("item_id");
        wait_for(reply);
        ASSERT_TRUE(reply.isValid());
        download_id = reply.argumentAt<0>();
        socket = reply.argumentAt<1>();
    }

    std::string data;
    auto app = QCoreApplication::instance();
    QSocketNotifier notifier(socket.fileDescriptor(), QSocketNotifier::Read);
    QObject::connect(
        &notifier, &QSocketNotifier::activated,
        [&data, app, &notifier](int fd) {
            char buf[1024];
            ssize_t n_read = read(fd, buf, sizeof(buf));
            if (n_read <= 0)
            {
                // Error or end of file
                notifier.setEnabled(false);
                app->quit();
            }
            else
            {
                data += string(buf, n_read);
            }
        });
    notifier.setEnabled(true);
    app->exec();
    auto reply = client_->FinishDownload(download_id);
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());

    // Also check that we got the expected data from the socket.
    EXPECT_EQ("Hello world", data);
}

TEST_F(ProviderInterfaceTest, download_short_read)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    QString download_id;
    QDBusUnixFileDescriptor socket;
    {
        auto reply = client_->Download("item_id");
        wait_for(reply);
        ASSERT_TRUE(reply.isValid());
        download_id = reply.argumentAt<0>();
        socket = reply.argumentAt<1>();
    }
    auto reply = client_->FinishDownload(download_id);
    wait_for(reply);
    ASSERT_TRUE(reply.isError());
    EXPECT_EQ(PROVIDER_ERROR + "LogicException", reply.error().name());
    EXPECT_EQ("Not all data read", reply.error().message());
}

TEST_F(ProviderInterfaceTest, finish_download_unknown)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->FinishDownload("no-such-download");
    wait_for(reply);
    ASSERT_TRUE(reply.isError());
    EXPECT_EQ(PROVIDER_ERROR + "UnknownException", reply.error().name());
    EXPECT_EQ("unknown exception thrown by provider: map::at", reply.error().message());
}

TEST_F(ProviderInterfaceTest, delete_)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->Delete("item_id");
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
}

TEST_F(ProviderInterfaceTest, move)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->Move("child_id", "new_parent_id", "New name");
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
    auto item = reply.value();
    EXPECT_EQ("child_id", item.item_id);
    EXPECT_EQ(QVector<QString>{ "new_parent_id" }, item.parent_ids);
    EXPECT_EQ("New name", item.name);
    EXPECT_EQ(ItemType::file, item.type);
}

TEST_F(ProviderInterfaceTest, copy)
{
    make_provider(unique_ptr<ProviderBase>(new TestProvider));

    auto reply = client_->Copy("child_id", "new_parent_id", "New name");
    wait_for(reply);
    ASSERT_TRUE(reply.isValid());
    auto item = reply.value();
    EXPECT_EQ("new_id", item.item_id);
    EXPECT_EQ(QVector<QString>{ "new_parent_id" }, item.parent_ids);
    EXPECT_EQ("New name", item.name);
    EXPECT_EQ(ItemType::file, item.type);
}


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    qDBusRegisterMetaType<unity::storage::internal::ItemMetadata>();
    qDBusRegisterMetaType<QList<unity::storage::internal::ItemMetadata>>();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
