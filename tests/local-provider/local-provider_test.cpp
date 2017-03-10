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
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

#include "../../src/local-provider/LocalProvider.h"

#include <unity/storage/provider/DownloadJob.h>
#include <unity/storage/provider/Exceptions.h>
#include <unity/storage/provider/Server.h>
#include <unity/storage/qt/client-api.h>
#include <utils/env_var_guard.h>
#include <utils/ProviderFixture.h>

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>

#include <chrono>
#include <regex>

using namespace unity::storage;
using namespace std;

namespace
{

int64_t nanosecs_now()
{
    return chrono::system_clock::now().time_since_epoch() / chrono::nanoseconds(1);
}

string const ROOT_DIR = TEST_DIR "/storage-framework";

class LocalProviderTest : public ProviderFixture
{
protected:
    void SetUp() override
    {
        boost::filesystem::remove_all(ROOT_DIR);

        ProviderFixture::SetUp();
        runtime_.reset(new qt::Runtime(connection()));
        acc_ = runtime_->make_test_account(service_connection_->baseService(), object_path());
    }

    void TearDown() override
    {
        runtime_.reset();
        ProviderFixture::TearDown();
    }

    unique_ptr<qt::Runtime> runtime_;
    qt::Account acc_;
};

constexpr int SIGNAL_WAIT_TIME = 30000;

}  // namespace

TEST_F(LocalProviderTest, basic)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    // Basic sanity check, get the root.
    unique_ptr<ItemListJob> j(acc_.roots());
    EXPECT_TRUE(j->isValid());
    QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
    ASSERT_TRUE(ready_spy.wait(SIGNAL_WAIT_TIME));
    ASSERT_EQ(1, ready_spy.count());
    auto arg = ready_spy.takeFirst();
    auto items = qvariant_cast<QList<Item>>(arg.at(0));
    ASSERT_EQ(1, items.size());

    // Check contents of returned item.
    auto root = items[0];
    EXPECT_TRUE(root.isValid());
    EXPECT_EQ(Item::Type::Root, root.type());
    EXPECT_EQ(TEST_DIR "/storage-framework", root.itemId());
    EXPECT_EQ("/", root.name());
    EXPECT_EQ("", root.etag());
    EXPECT_EQ(QList<QString>(), root.parentIds());
    qDebug() << root.lastModifiedTime();
    EXPECT_TRUE(root.lastModifiedTime().isValid());
    EXPECT_EQ(acc_, root.account());

    ASSERT_EQ(5, root.metadata().size());
    auto free_space_bytes = root.metadata().value("free_space_bytes").toULongLong();
    cout << "free_space_bytes: " << free_space_bytes << endl;
    EXPECT_GT(free_space_bytes, 0);
    auto used_space_bytes = root.metadata().value("used_space_bytes").toULongLong();
    cout << "used_space_bytes: " << used_space_bytes << endl;
    EXPECT_GT(used_space_bytes, 0);
    auto content_type = root.metadata().value("content_type").toString();
    EXPECT_EQ("inode/directory", content_type);
    auto writable = root.metadata().value("writable").toBool();
    EXPECT_TRUE(writable);

    // yyyy-mm-ddThh:mm:ssZ
    string const date_time_fmt = "^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9]Z$";
    string mtime = root.metadata().value("last_modified_time").toString().toStdString();
    cout << "last_modified_time: " << mtime << endl;
    regex re(date_time_fmt);
    EXPECT_TRUE(regex_match(mtime, re));
}

TEST_F(LocalProviderTest, xdg_dir)
{
    // Check that we are paying attention to XDG_DATA_HOME.

    using namespace unity::storage::qt;

    EnvVarGuard env1("STORAGE_FRAMEWORK_ROOT", nullptr);
    EnvVarGuard env2("XDG_DATA_HOME", "/tmp");
    ::rmdir("/tmp/storage-framework");

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    unique_ptr<ItemListJob> j(acc_.roots());
    EXPECT_TRUE(j->isValid());
    QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
    ASSERT_TRUE(ready_spy.wait(SIGNAL_WAIT_TIME));
    ASSERT_EQ(1, ready_spy.count());
    auto arg = ready_spy.takeFirst();
    auto items = qvariant_cast<QList<Item>>(arg.at(0));
    ASSERT_EQ(1, items.size());

    auto root = items[0];
    EXPECT_EQ("/tmp/storage-framework", root.itemId());
}

TEST(LocalProviderExceptions, constructor_exceptions)
{
    // These tests cause the constructor to throw, so we instantiate the provider directly.

    {
        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", "/no_such_dir");

        try
        {
            LocalProvider();
            FAIL();
        }
        catch (provider::ResourceException const& e)
        {
            EXPECT_STREQ("ResourceException: LocalProvider(): Cannot stat /no_such_dir: No such file or directory",
                         e.what());
        }
    }

    {
        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", TEST_DIR "/Makefile");

        try
        {
            LocalProvider();
            FAIL();
        }
        catch (provider::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: LocalProvider(): Environment variable "
                         "STORAGE_FRAMEWORK_ROOT must denote a directory",
                         e.what());
        }
    }

    {
        ::mkdir(TEST_DIR "/noperm", 0555);

        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", TEST_DIR "/noperm");

        try
        {
            LocalProvider();
            ::rmdir(TEST_DIR "/noperm");
            FAIL();
        }
        catch (provider::ResourceException const& e)
        {
            EXPECT_STREQ("ResourceException: LocalProvider(): Cannot create " TEST_DIR "/noperm: Permission denied",
                         e.what());
        }
        ::rmdir(TEST_DIR "/noperm");
    }

}

#if 0
TEST_F(LocalProviderTest, create_folder)
{
    {
        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", TEST_DIR);

        auto p = make_shared<LocalProvider>();

        Item root;
        {
            auto fut = p->roots({}, Context());
            auto roots = fut.get();
            root = roots.at(0);
        }

        Item child;
        {
            auto fut = p->create_folder(root.item_id, "child", {}, Context());
            child = fut.get();
        }

        EXPECT_EQ("child", child.name);
        ASSERT_EQ(1, child.parent_ids.size());
        EXPECT_EQ(root.item_id, child.parent_ids[0]);
        EXPECT_EQ("", child.etag);
        EXPECT_EQ(ItemType::folder, child.type);

        ASSERT_EQ(5, child.metadata.size());

        struct stat st;
        ASSERT_EQ(0, stat(child.item_id.c_str(), &st));
        EXPECT_TRUE(S_ISDIR(st.st_mode));
    }

    {
        // Bad directory names.

        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", TEST_DIR);

        auto p = make_shared<LocalProvider>();

        Item root;
        {
            auto fut = p->roots({}, Context());
            auto roots = fut.get();
            root = roots.at(0);
        }

        try
        {
            // Empty name
            auto fut = p->create_folder(root.item_id, "", {}, Context());
            fut.get();
            FAIL();
        }
        catch (InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: create_folder(): invalid name: \"\"", e.what());
        }

        try
        {
            // "."
            auto fut = p->create_folder(root.item_id, ".", {}, Context());
            fut.get();
            FAIL();
        }
        catch (InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: create_folder(): invalid name: \".\"", e.what());
        }

        try
        {
            // ".."
            auto fut = p->create_folder(root.item_id, "..", {}, Context());
            fut.get();
            FAIL();
        }
        catch (InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: create_folder(): invalid name: \"..\"", e.what());
        }

        try
        {
            // Trailing slash
            auto fut = p->create_folder(root.item_id, "abc/", {}, Context());
            fut.get();
            FAIL();
        }
        catch (InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: create_folder(): name \"abc/\" cannot contain a slash",
                         e.what());
        }

        try
        {
            // Leading slash
            auto fut = p->create_folder(root.item_id, "/abc", {}, Context());
            fut.get();
            FAIL();
        }
        catch (InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: create_folder(): name \"/abc\" cannot contain a slash",
                         e.what());
        }

        try
        {
            // Reserved name
            auto fut = p->create_folder(root.item_id, ".storage-framework", {}, Context());
            fut.get();
            FAIL();
        }
        catch (InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: create_folder(): names beginning with "
                         "\".storage-framework\" are reserved",
                         e.what());
        }

        // Force permission error.
        ASSERT_EQ(0, chmod(ROOT_DIR.c_str(), 0555));
        try
        {
            auto fut = p->create_folder(root.item_id, "abc", {}, Context());
            fut.get();
            chmod(ROOT_DIR.c_str(), 0755);
            FAIL();
        }
        catch (PermissionException const& e)
        {
            chmod(ROOT_DIR.c_str(), 0755);
            EXPECT_EQ(string("PermissionException: create_folder(): \"") + ROOT_DIR + "/abc\": "
                      "boost::filesystem::create_directory: Permission denied: \"" + ROOT_DIR + "/abc\"",
                      e.what());
        }

        // Non-existent parent
        try
        {
            auto fut = p->create_folder(ROOT_DIR + "/no_such_parent", "abc", {}, Context());
            fut.get();
            FAIL();
        }
        catch (NotExistsException const& e)
        {
            EXPECT_EQ(string("NotExistsException: create_folder(): \"") + ROOT_DIR + "/no_such_parent\": "
                      "boost::filesystem::canonical: No such file or directory: \"" + ROOT_DIR + "/no_such_parent\"",
                      e.what());
        }

        // Directory exists.
        try
        {
            auto fut = p->create_folder(ROOT_DIR, "child", {}, Context());
            fut.get();
            FAIL();
        }
        catch (ExistsException const& e)
        {
            string item_path = ROOT_DIR + "/child";
            EXPECT_EQ("ExistsException: create_folder(): \"" + item_path + "\" exists already", e.what());
            EXPECT_EQ(item_path, e.native_identity());
            EXPECT_EQ("child", e.name());
        }

        // Parent ID outside root.
        try
        {
            auto fut = p->create_folder(ROOT_DIR + "/..", "abc", {}, Context());
            fut.get();
            FAIL();
        }
        catch (InvalidArgumentException const& e)
        {
            EXPECT_EQ(string("InvalidArgumentException: create_folder(): invalid id: \"") + ROOT_DIR + "/..\"", e.what());
        }

        // Parent ID different from root.
        try
        {
            auto fut = p->create_folder("/tmp", "abc", {}, Context());
            fut.get();
            FAIL();
        }
        catch (InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: create_folder(): invalid id: \"/tmp\"", e.what());
        }
    }
}

TEST_F(LocalProviderTest, metadata)
{
    {
        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", TEST_DIR);

        auto p = make_shared<LocalProvider>();

        // Make a file
        auto cmd = string("echo hello >") + ROOT_DIR + "/hello";
        auto start_time = nanosecs_now();
        ASSERT_EQ(0, system(cmd.c_str()));

        Item hello;
        {
            auto fut = p->metadata(ROOT_DIR + "/hello", {}, Context());
            hello = fut.get();
        }

        EXPECT_EQ("hello", hello.name);
        ASSERT_EQ(1, hello.parent_ids.size());
        EXPECT_EQ(ROOT_DIR, hello.parent_ids[0]);
        EXPECT_EQ(ItemType::file, hello.type);

        ASSERT_EQ(6, hello.metadata.size());
        auto free_space_bytes = boost::get<int64_t>(hello.metadata.at("free_space_bytes"));
        cout << "free_space_bytes: " << free_space_bytes << endl;
        EXPECT_GT(free_space_bytes, 0);
        auto used_space_bytes = boost::get<int64_t>(hello.metadata.at("used_space_bytes"));
        cout << "used_space_bytes: " << used_space_bytes << endl;
        EXPECT_GT(used_space_bytes, 0);
        auto content_type = boost::get<string>(hello.metadata.at("content_type"));
        EXPECT_EQ("application/octet-stream", content_type);
        auto writable = boost::get<int64_t>(hello.metadata.at("writable"));
        EXPECT_TRUE(writable);
        auto size = boost::get<int64_t>(hello.metadata.at("size_in_bytes"));
        EXPECT_EQ(6, size);

        // yyyy-mm-ddThh:mm:ssZ
        string const date_time_fmt = "^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9]Z$";
        string date_time = boost::get<string>(hello.metadata.at("last_modified_time"));
        cout << "last_modified_time: " << date_time << endl;
        regex re(date_time_fmt);
        EXPECT_TRUE(regex_match(date_time, re));

        // Check that the file was modified in the last two seconds.
        // Because the system clock can tick a lot more frequently than the file system time stamp,
        // we allow the mtime to be up to one second *earlier* than the time we started the operation.
        char* end;
        int64_t mtime = strtoll(hello.etag.c_str(), &end, 10);
        EXPECT_LE(start_time - 1000000000, mtime);
        EXPECT_LT(mtime, start_time + 2000000000);
    }

    {
        // Force permission error.

        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", TEST_DIR);

        auto p = make_shared<LocalProvider>();

        ASSERT_EQ(0, chmod((ROOT_DIR).c_str(), 0444));

        try
        {
            auto fut = p->metadata(ROOT_DIR + "/hello", {}, Context());
            fut.get();
            chmod((ROOT_DIR).c_str(), 0755);
            FAIL();
        }
        catch (PermissionException const& e)
        {
            chmod((ROOT_DIR).c_str(), 0755);
            EXPECT_EQ(string("PermissionException: metadata(): \"") + ROOT_DIR + "/hello\": "
                      "boost::filesystem::canonical: Permission denied: \"" + ROOT_DIR + "/hello\"",
                      e.what());
        }
    }

    {
        // Try with a named pipe (neither file nor folder).

        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", TEST_DIR);

        auto p = make_shared<LocalProvider>();

        // Make a named pipe.
        ASSERT_EQ(0, mknod((ROOT_DIR + "/pipe").c_str(), S_IFIFO|06666, 0));

        try
        {
            auto fut = p->metadata(ROOT_DIR + "/pipe", {}, Context());
            fut.get();
            FAIL();
        }
        catch (NotExistsException const& e)
        {
            EXPECT_EQ(string("NotExistsException: metadata(): \"") + ROOT_DIR + "/pipe\" is neither a file nor a folder",
                      e.what());
        }
    }
}

TEST_F(LocalProviderTest, download)
{
#if 0
    {
        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", TEST_DIR);

        auto p = make_shared<LocalProvider>();

        // Make a file
        auto const file_path = ROOT_DIR + "/hello";
        auto cmd = string("echo hello >") + file_path;
        ASSERT_EQ(0, system(cmd.c_str()));
        chmod(file_path.c_str(), 0000);

        try
        {
            auto job = p->download(file_path, "", Context());
            FAIL();
        }
        catch (NotExistsException const& e)
        {
            EXPECT_EQ(string("NotExistsException: download(): \"") + file_path + "\": "
                         "boost::filesystem::canonical: No such file or directory: \"/bad_id\"",
                         e.what());
        }
    }
#endif

    {
        EnvVarGuard env("STORAGE_FRAMEWORK_ROOT", TEST_DIR);

        auto p = make_shared<LocalProvider>();

        try
        {
            auto job = p->download("/bad_id", "", Context());
            FAIL();
        }
        catch (NotExistsException const& e)
        {
            EXPECT_STREQ("NotExistsException: download(): \"/bad_id\": "
                         "boost::filesystem::canonical: No such file or directory: \"/bad_id\"",
                         e.what());
        }
    }
}
#endif

int main(int argc, char** argv)
{
    setenv("LANG", "C", true);
    setenv("STORAGE_FRAMEWORK_ROOT", TEST_DIR, true);

    // Test test fixture repeatedly creates and tears down the dbus connection.
    // The provider calls g_file_new_for_path() which talks to the GVfs backend
    // via dbus. If the dbus connection disappears, that causes GIO to send a
    // a SIGTERM, killing the test.
    // Setting GIO_USE_VFS variable to "local" disables sending the signal.
    setenv("GIO_USE_VFS", "local", true);

    QCoreApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    int rc = RUN_ALL_TESTS();

    if (rc == 0)
    {
        boost::system::error_code ec;
        boost::filesystem::remove_all(ROOT_DIR, ec);
    }

    return rc;
}
