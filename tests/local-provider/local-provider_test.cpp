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

#include "../../src/local-provider/LocalDownloadJob.h"
#include "../../src/local-provider/LocalProvider.h"
#include "../../src/local-provider/LocalUploadJob.h"

#include <unity/storage/provider/DownloadJob.h>
#include <unity/storage/provider/Exceptions.h>
#include <unity/storage/provider/Server.h>
#include <unity/storage/qt/client-api.h>
#include <utils/env_var_guard.h>
#include <utils/ProviderFixture.h>

#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>

#include <chrono>
#include <regex>

#include <fcntl.h>

using namespace unity::storage;
using namespace std;

namespace
{

int64_t nanosecs_now()
{
    return chrono::system_clock::now().time_since_epoch() / chrono::nanoseconds(1);
}

class LocalProviderTest : public ProviderFixture
{
protected:
    void SetUp() override
    {
        tmp_dir_.reset(new QTemporaryDir(TEST_DIR "/data.XXXXXX"));
        ASSERT_TRUE(tmp_dir_->isValid());
        setenv("SF_LOCAL_PROVIDER_ROOT", ROOT_DIR().c_str(), true);

        ProviderFixture::SetUp();
        runtime_.reset(new qt::Runtime(connection()));
        acc_ = runtime_->make_test_account(service_connection_->baseService(), object_path());
    }

    void TearDown() override
    {
        runtime_.reset();
        ProviderFixture::TearDown();
        if (HasFailure())
        {
            tmp_dir_->setAutoRemove(false);
        }
        tmp_dir_.reset();
    }

    std::string ROOT_DIR() const
    {
        return tmp_dir_->path().toStdString();
    }

    std::unique_ptr<QTemporaryDir> tmp_dir_;
    unique_ptr<qt::Runtime> runtime_;
    qt::Account acc_;
};

constexpr int SIGNAL_WAIT_TIME = 30000;

template <typename Job>
void wait(Job* job)
{
    QSignalSpy spy(job, &Job::statusChanged);
    while (job->status() == Job::Loading)
    {
        if (!spy.wait(SIGNAL_WAIT_TIME))
        {
            throw runtime_error("Wait for statusChanged signal timed out");
        }
    }
}

qt::Item get_root(qt::Account const& account)
{
    unique_ptr<qt::ItemListJob> j(account.roots());
    assert(j->isValid());
    QSignalSpy ready_spy(j.get(), &qt::ItemListJob::itemsReady);
    assert(ready_spy.wait(SIGNAL_WAIT_TIME));
    auto arg = ready_spy.takeFirst();
    auto items = qvariant_cast<QList<qt::Item>>(arg.at(0));
    assert(items.size() == 1);
    return items[0];
}

QList<qt::Item> get_items(qt::ItemListJob *job)
{
    QList<qt::Item> items;
    auto connection = QObject::connect(
        job, &qt::ItemListJob::itemsReady,
        [&](QList<qt::Item> const& new_items)
        {
            items.append(new_items);
        });
    try
    {
        wait(job);
    }
    catch (...)
    {
        QObject::disconnect(connection);
        throw;
    }
    QObject::disconnect(connection);
    return items;
}

const string file_contents =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
    "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut "
    "enim ad minim veniam, quis nostrud exercitation ullamco laboris "
    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor "
    "in reprehenderit in voluptate velit esse cillum dolore eu fugiat "
    "nulla pariatur. Excepteur sint occaecat cupidatat non proident, "
    "sunt in culpa qui officia deserunt mollit anim id est laborum.\n";

}  // namespace

TEST(Directories, env_vars)
{
    // These tests cause the constructor to throw, so we instantiate the provider directly.

    {
        EnvVarGuard env("SF_LOCAL_PROVIDER_ROOT", "/no_such_dir");

        try
        {
            LocalProvider();
            FAIL();
        }
        catch (provider::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: LocalProvider(): Environment variable "
                         "SF_LOCAL_PROVIDER_ROOT must denote an existing directory",
                         e.what());
        }
    }

    {
        EnvVarGuard env("SF_LOCAL_PROVIDER_ROOT", TEST_DIR "/Makefile");

        try
        {
            LocalProvider();
            FAIL();
        }
        catch (provider::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("InvalidArgumentException: LocalProvider(): Environment variable "
                         "SF_LOCAL_PROVIDER_ROOT must denote an existing directory",
                         e.what());
        }
    }

    {
        string const dir = TEST_DIR "/noperm";

        mkdir(dir.c_str(), 0555);
        ASSERT_EQ(0, chmod(dir.c_str(), 0555));  // In case dir was there already.

        EnvVarGuard env1("SF_LOCAL_PROVIDER_ROOT", nullptr);
        EnvVarGuard env2("XDG_DATA_HOME", dir.c_str());

        using namespace boost::filesystem;

        try
        {
            LocalProvider();
            ASSERT_EQ(0, chmod(dir.c_str(), 0775));
            remove_all(dir);
            FAIL();
        }
        catch (provider::PermissionException const& e)
        {
            EXPECT_EQ(string("PermissionException: LocalProvider(): \"") + dir + "/storage-framework\": "
                      "boost::filesystem::create_directories: Permission denied: \"" + dir + "/storage-framework\"",
                      e.what());
        }

        ASSERT_EQ(0, chmod(dir.c_str(), 0775));
        ASSERT_TRUE(remove_all(dir));

        // Try again, which must succeed now (for coverage).
        LocalProvider();
        ASSERT_TRUE(is_directory(dir + "/storage-framework/local"));
        ASSERT_TRUE(remove_all(dir));
    }

    {
        string const dir = TEST_DIR "/snap_user_common";
        mkdir(dir.c_str(), 0775);

        EnvVarGuard env1("SF_LOCAL_PROVIDER_ROOT", nullptr);
        EnvVarGuard env2("SNAP_USER_COMMON", dir.c_str());

        using namespace boost::filesystem;

        LocalProvider();
        ASSERT_TRUE(exists(dir + "/storage-framework/local"));
        ASSERT_TRUE(remove_all(dir));
    }
}

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
    EXPECT_EQ(ROOT_DIR(), root.itemId().toStdString());
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

TEST_F(LocalProviderTest, create_folder)
{
    {
        using namespace unity::storage::qt;

        set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

        auto root = get_root(acc_);
        unique_ptr<ItemJob> job(root.createFolder("child"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();

        Item child = job->item();
        EXPECT_EQ(ROOT_DIR() + "/child", child.itemId().toStdString());
        EXPECT_EQ("child", child.name().toStdString());
        ASSERT_EQ(1, child.parentIds().size());
        EXPECT_EQ(ROOT_DIR(), child.parentIds().at(0).toStdString());
        EXPECT_EQ("", child.etag());
        EXPECT_EQ(Item::Type::Folder, child.type());
        EXPECT_EQ(5, child.metadata().size());

        struct stat st;
        ASSERT_EQ(0, stat(child.itemId().toStdString().c_str(), &st));
        EXPECT_TRUE(S_ISDIR(st.st_mode));

        // Again, to get coverage for a StorageException caught in invoke_async().
        job.reset(root.createFolder("child"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Error, job->status()) << job->error().errorString().toStdString();
        EXPECT_EQ(string("Exists: create_folder(): \"") + ROOT_DIR() + "/child\" exists already",
                  job->error().errorString().toStdString());

        // Again, without write permission on the root dir, to get coverage for a filesystem_error in invoke_async().
        ASSERT_EQ(0, ::rmdir((ROOT_DIR() + "/child").c_str()));
        ASSERT_EQ(0, ::chmod(ROOT_DIR().c_str(), 0555));
        job.reset(root.createFolder("child"));
        wait(job.get());
        ::chmod(ROOT_DIR().c_str(), 0755);
        ASSERT_EQ(ItemJob::Error, job->status()) << job->error().errorString().toStdString();
        EXPECT_EQ(string("PermissionDenied: create_folder(): \"") + ROOT_DIR()
                  + "/child\": boost::filesystem::create_directory: Permission denied: \"" + ROOT_DIR() + "/child\"",
                  job->error().errorString().toStdString());
    }
}

TEST_F(LocalProviderTest, delete_item)
{
    {
        using namespace unity::storage::qt;

        set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

        auto root = get_root(acc_);
        unique_ptr<ItemJob> job(root.createFolder("child"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();

        Item child = job->item();
        unique_ptr<VoidJob> delete_job(child.deleteItem());
        wait(delete_job.get());
        ASSERT_EQ(ItemJob::Finished, delete_job->status()) << delete_job->error().errorString().toStdString();

        struct stat st;
        ASSERT_EQ(-1, stat(child.itemId().toStdString().c_str(), &st));
        EXPECT_EQ(ENOENT, errno);
    }
}

TEST_F(LocalProviderTest, delete_item_noperm)
{
    {
        using namespace unity::storage::qt;

        set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

        auto root = get_root(acc_);
        unique_ptr<ItemJob> job(root.createFolder("child"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();
    }
}

TEST_F(LocalProviderTest, delete_root)
{
    // Client-side API does not allow us to try to delete the root, so we talk to the provider directly.
    auto p = make_shared<LocalProvider>();

    auto fut = p->delete_item(ROOT_DIR(), provider::Context());
    try
    {
        fut.get();
        FAIL();
    }
    catch (provider::LogicException const& e)
    {
        EXPECT_STREQ("LogicException: delete_item(): cannot delete root", e.what());
    }
}

TEST_F(LocalProviderTest, metadata)
{
    // Client-side API does not call the Metadata DBus method (except as part of parents()),
    // so we talk to the provider directly.
    auto p = make_shared<LocalProvider>();

    auto fut = p->metadata(ROOT_DIR(), {}, provider::Context());
    auto item = fut.get();
    EXPECT_EQ(5, item.metadata.size());

    // Again, to get coverage for the "not file or folder" case in make_item().
    ASSERT_EQ(0, mknod((ROOT_DIR() + "/pipe").c_str(), S_IFIFO | 06666, 0));
    try
    {
        auto fut = p->metadata(ROOT_DIR() + "/pipe", {}, provider::Context());
        fut.get();
        FAIL();
    }
    catch (provider::NotExistsException const& e)
    {
        EXPECT_EQ(string("NotExistsException: metadata(): \"") + ROOT_DIR() + "/pipe\" is neither a file nor a folder",
                  e.what());
    }
}

TEST_F(LocalProviderTest, lookup)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto root = get_root(acc_);
    {
        unique_ptr<ItemJob> job(root.createFolder("child"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();
    }

    unique_ptr<ItemListJob> job(root.lookup("child"));
    auto items = get_items(job.get());
    ASSERT_EQ(1, items.size());
    auto child = items.at(0);

    EXPECT_EQ(ROOT_DIR() + "/child", child.itemId().toStdString());
    EXPECT_EQ("child", child.name().toStdString());
    ASSERT_EQ(1, child.parentIds().size());
    EXPECT_EQ(ROOT_DIR(), child.parentIds().at(0).toStdString());
    EXPECT_EQ("", child.etag());
    EXPECT_EQ(Item::Type::Folder, child.type());
    EXPECT_EQ(5, child.metadata().size());

    // Remove the child again and try the lookup once more.
    ASSERT_EQ(0, rmdir((ROOT_DIR() + "/child").c_str()));

    job.reset(root.lookup("child"));
    wait(job.get());
    EXPECT_EQ(ItemJob::Error, job->status());
    EXPECT_EQ(string("NotExists: lookup(): \"") + ROOT_DIR() + "/child\": boost::filesystem::canonical: "
              + "No such file or directory: \"" + ROOT_DIR() + "/child\"",
              job->error().errorString().toStdString());
}

TEST_F(LocalProviderTest, list)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto root = get_root(acc_);
    {
        unique_ptr<ItemJob> job(root.createFolder("child"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();
    }

    // Make a weird item that will be ignored (for coverage).
    ASSERT_EQ(0, mknod((ROOT_DIR() + "/pipe").c_str(), S_IFIFO | 06666, 0));

    // Make a file that starts with the temp file prefix (for coverage).
    int fd = creat((ROOT_DIR() + "/.storage-framework").c_str(), 0755);
    ASSERT_GT(fd, 0);
    close(fd);

    unique_ptr<ItemListJob> job(root.list());
    auto items = get_items(job.get());
    ASSERT_EQ(1, items.size());
    auto child = items.at(0);

    EXPECT_EQ(ROOT_DIR() + "/child", child.itemId().toStdString());
    EXPECT_EQ("child", child.name().toStdString());
    ASSERT_EQ(1, child.parentIds().size());
    EXPECT_EQ(ROOT_DIR(), child.parentIds().at(0).toStdString());
    EXPECT_EQ("", child.etag());
    EXPECT_EQ(Item::Type::Folder, child.type());
    EXPECT_EQ(5, child.metadata().size());
}

void make_hierarchy(string const& root_dir)
{
    // Make a small tree so we have something to test with for move() and copy().
    ASSERT_EQ(0, mkdir((root_dir + "/a").c_str(), 0755));
    ASSERT_EQ(0, mkdir((root_dir + "/a/b").c_str(), 0755));
    string cmd = string("echo hello >") + root_dir + "/hello";
    ASSERT_EQ(0, system(cmd.c_str()));
    cmd = string("echo text >") + root_dir + "/a/foo.txt";
    ASSERT_EQ(0, system(cmd.c_str()));
    ASSERT_EQ(0, mknod((root_dir + "/a/pipe").c_str(), S_IFIFO | 06666, 0));
    ASSERT_EQ(0, mkdir((root_dir + "/a/.storage-framework-").c_str(), 0755));
    ASSERT_EQ(0, mkdir((root_dir + "/a/b/.storage-framework-").c_str(), 0755));
    ASSERT_EQ(0, mknod((root_dir + "/a/b/pipe").c_str(), S_IFIFO | 06666, 0));
}

TEST_F(LocalProviderTest, move)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto start_time = nanosecs_now();

    make_hierarchy(ROOT_DIR());

    auto root = get_root(acc_);

    qt::Item hello;
    {
        unique_ptr<ItemListJob> job(root.lookup("hello"));
        auto items = get_items(job.get());
        ASSERT_EQ(ItemListJob::Finished, job->status()) << job->error().errorString().toStdString();
        ASSERT_EQ(1, items.size());
        hello = items.at(0);
    }

    struct stat st;
    ASSERT_EQ(0, stat(hello.itemId().toStdString().c_str(), &st));
    auto old_ino = st.st_ino;

    // Check metadata.
    EXPECT_EQ("hello", hello.name());
    ASSERT_EQ(1, hello.parentIds().size());
    EXPECT_EQ(ROOT_DIR(), hello.parentIds().at(0).toStdString());
    EXPECT_EQ(Item::Type::File, hello.type());

    ASSERT_EQ(6, hello.metadata().size());
    auto free_space_bytes = hello.metadata().value("free_space_bytes").toLongLong();
    cout << "free_space_bytes: " << free_space_bytes << endl;
    EXPECT_GT(free_space_bytes, 0);
    auto used_space_bytes = hello.metadata().value("used_space_bytes").toLongLong();
    cout << "used_space_bytes: " << used_space_bytes << endl;
    EXPECT_GT(used_space_bytes, 0);
    auto content_type = hello.metadata().value("content_type").toString();
    EXPECT_EQ("application/octet-stream", content_type);
    auto writable = hello.metadata().value("writable").toBool();
    EXPECT_TRUE(writable);
    auto size = hello.metadata().value("size_in_bytes").toLongLong();
    EXPECT_EQ(6, size);

    // yyyy-mm-ddThh:mm:ssZ
    string const date_time_fmt = "^[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9]Z$";
    string date_time = hello.metadata().value("last_modified_time").toString().toStdString();
    cout << "last_modified_time: " << date_time << endl;
    regex re(date_time_fmt);
    EXPECT_TRUE(regex_match(date_time, re));

    // Check that the file was modified in the last two seconds.
    // Because the system clock can tick a lot more frequently than the file system time stamp,
    // we allow the mtime to be up to one second *earlier* than the time we started the operation.
    string mtime_str = hello.etag().toStdString();
    char* end;
    int64_t mtime = strtoll(mtime_str.c_str(), &end, 10);
    EXPECT_LE(start_time - 1000000000, mtime);
    EXPECT_LT(mtime, start_time + 2000000000);

    // Move hello -> world
    qt::Item world;
    {
        unique_ptr<ItemJob> job(hello.move(root, "world"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();
        world = job->item();
    }
    EXPECT_FALSE(boost::filesystem::exists(hello.itemId().toStdString()));
    EXPECT_EQ(ROOT_DIR() + "/world", world.itemId().toStdString());

    ASSERT_EQ(0, stat(world.itemId().toStdString().c_str(), &st));
    auto new_ino = st.st_ino;
    EXPECT_EQ(old_ino, new_ino);

    // For coverage: try moving world -> a (which must fail)
    unique_ptr<ItemJob> job(world.move(root, "a"));
    wait(job.get());
    ASSERT_EQ(ItemJob::Error, job->status()) << job->error().errorString().toStdString();
    EXPECT_EQ(string("Exists: move(): \"") + ROOT_DIR() + "/a\" exists already",
              job->error().errorString().toStdString());
}

TEST_F(LocalProviderTest, copy_file)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    make_hierarchy(ROOT_DIR());

    auto root = get_root(acc_);

    // Copy hello -> world
    qt::Item hello;
    {
        unique_ptr<ItemListJob> job(root.lookup("hello"));
        auto items = get_items(job.get());
        ASSERT_EQ(ItemListJob::Finished, job->status()) << job->error().errorString().toStdString();
        ASSERT_EQ(1, items.size());
        hello = items.at(0);
    }

    struct stat st;
    ASSERT_EQ(0, stat(hello.itemId().toStdString().c_str(), &st));
    auto old_ino = st.st_ino;

    qt::Item world;
    {
        unique_ptr<ItemJob> job(hello.copy(root, "world"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();
        world = job->item();
    }
    EXPECT_TRUE(boost::filesystem::exists(hello.itemId().toStdString()));
    EXPECT_EQ(ROOT_DIR() + "/world", world.itemId().toStdString());

    ASSERT_EQ(0, stat(world.itemId().toStdString().c_str(), &st));
    auto new_ino = st.st_ino;
    EXPECT_NE(old_ino, new_ino);
}

TEST_F(LocalProviderTest, copy_tree)
{
    using namespace unity::storage::qt;
    using namespace boost::filesystem;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    make_hierarchy(ROOT_DIR());

    auto root = get_root(acc_);

    // Copy a -> c
    qt::Item a;
    {
        unique_ptr<ItemListJob> job(root.lookup("a"));
        auto items = get_items(job.get());
        ASSERT_EQ(ItemListJob::Finished, job->status()) << job->error().errorString().toStdString();
        ASSERT_EQ(1, items.size());
        a = items.at(0);
    }

    qt::Item c;
    {
        unique_ptr<ItemJob> job(a.copy(root, "c"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();
        c = job->item();
    }
    EXPECT_TRUE(exists(c.itemId().toStdString()));

    // Check that we only copied regular files and directories, but not a pipe or anything starting with
    // the temp file prefix.
    EXPECT_TRUE(exists(ROOT_DIR() + "/c/b"));
    EXPECT_TRUE(exists(ROOT_DIR() + "/c/foo.txt"));
    EXPECT_FALSE(exists(ROOT_DIR() + "/c/pipe"));
    EXPECT_FALSE(exists(ROOT_DIR() + "/c/storage-framework-"));
    EXPECT_FALSE(exists(ROOT_DIR() + "/c/b/pipe"));
    EXPECT_FALSE(exists(ROOT_DIR() + "/c/b/storage-framework-"));

    // Copy c -> a. This must fail because a exists.
    {
        unique_ptr<ItemJob> job(c.copy(root, "a"));
        wait(job.get());
        ASSERT_EQ(ItemJob::Error, job->status()) << job->error().errorString().toStdString();
        EXPECT_EQ(string("Exists: copy(): \"") + ROOT_DIR() + "/a\" exists already",
                  job->error().errorString().toStdString());
    }
}

TEST_F(LocalProviderTest, download)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    int const segments = 10000;
    string large_contents;
    large_contents.reserve(file_contents.size() * segments);
    for (int i = 0; i < segments; i++)
    {
        large_contents += file_contents;
    }
    string const full_path = ROOT_DIR() + "/foo.txt";
    {
        int fd = open(full_path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
        ASSERT_GT(fd, 0);
        ASSERT_EQ(ssize_t(large_contents.size()), write(fd, &large_contents[0], large_contents.size())) << strerror(errno);
        ASSERT_EQ(0, close(fd));
    }

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    EXPECT_TRUE(job->isValid());

    auto file = job->item();
    unique_ptr<Downloader> downloader(file.createDownloader(Item::ErrorIfConflict));

    int64_t n_read = 0;
    QObject::connect(downloader.get(), &QIODevice::readyRead,
                     [&]() {
                         auto bytes = downloader->readAll();
                         string const expected = large_contents.substr(n_read, bytes.size());
                         EXPECT_EQ(expected, bytes.toStdString());
                         n_read += bytes.size();
                     });
    QSignalSpy read_finished_spy(downloader.get(), &QIODevice::readChannelFinished);
    ASSERT_TRUE(read_finished_spy.wait(SIGNAL_WAIT_TIME));

    QSignalSpy status_spy(downloader.get(), &Downloader::statusChanged);
    downloader->close();
    while (downloader->status() == Downloader::Ready)
    {
        ASSERT_TRUE(status_spy.wait(SIGNAL_WAIT_TIME));
    }
    ASSERT_EQ(Downloader::Finished, downloader->status()) << downloader->error().errorString().toStdString();

    EXPECT_EQ(int64_t(large_contents.size()), n_read);
}

TEST_F(LocalProviderTest, download_short_read)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    int const segments = 10000;
    string const full_path = ROOT_DIR() + "/foo.txt";
    {
        int fd = open(full_path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
        ASSERT_GT(fd, 0);
        for (int i = 0; i < segments; i++)
        {
            ASSERT_EQ(ssize_t(file_contents.size()), write(fd, &file_contents[0], file_contents.size())) << strerror(errno);
        }
        ASSERT_EQ(0, close(fd));
    }

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();

    auto file = job->item();
    unique_ptr<Downloader> downloader(file.createDownloader(Item::ErrorIfConflict));

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
    while (downloader->status() == Downloader::Loading)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }

    downloader->close();
    while (downloader->status() == Downloader::Ready)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    ASSERT_EQ(Downloader::Error, downloader->status()) << downloader->error().errorString().toStdString();

    auto error = downloader->error();
    EXPECT_EQ(qt::StorageError::LogicError, error.type());
    cout << error.message().toStdString() << endl;
    EXPECT_TRUE(boost::starts_with(error.message().toStdString(),
                                   "finish() method called too early, file \""
                                   + full_path + "\" has size 4460000 but only"));
}

TEST_F(LocalProviderTest, download_etag_mismatch)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    string const full_path = ROOT_DIR() + "/foo.txt";
    string cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();

    auto file = job->item();

    sleep(1);
    cmd = string("touch ") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<Downloader> downloader(file.createDownloader(Item::ErrorIfConflict));

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
    while (downloader->status() != Downloader::Error)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }

    auto error = downloader->error();
    EXPECT_EQ(qt::StorageError::Conflict, error.type());
    EXPECT_EQ("download(): etag mismatch", error.message().toStdString());
}

TEST_F(LocalProviderTest, download_wrong_file_type)
{
    // We can't try a download for a directory via the client API, so we use the LocalDownloadJob directly.

    auto p = make_shared<LocalProvider>();

    string const dir = ROOT_DIR() + "/dir";
    ASSERT_EQ(0, mkdir(dir.c_str(), 0755));

    try
    {
        LocalDownloadJob(p, dir, "some_etag");
        FAIL();
    }
    catch (provider::InvalidArgumentException const& e)
    {
        EXPECT_EQ(string("InvalidArgumentException: download(): \"" + dir + "\" is not a file"), e.what());
    }
}

TEST_F(LocalProviderTest, download_no_permission)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    string const full_path = ROOT_DIR() + "/foo.txt";
    string cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    ASSERT_EQ(ItemJob::Finished, job->status()) << job->error().errorString().toStdString();

    auto file = job->item();

    ASSERT_EQ(0, chmod(full_path.c_str(), 0244));

    unique_ptr<Downloader> downloader(file.createDownloader(Item::ErrorIfConflict));

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
    while (downloader->status() != Downloader::Error)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }

    auto error = downloader->error();
    EXPECT_EQ(qt::StorageError::ResourceError, error.type());
    EXPECT_EQ(string("download(): : cannot open \"") + full_path + "\": Permission denied (QFileDevice::FileError = 5)",
              error.message().toStdString());
}

TEST_F(LocalProviderTest, download_no_such_file)
{
    // We can't try a download for a non-existent file via the client API, so we use the LocalDownloadJob directly.

    auto p = make_shared<LocalProvider>();

    try
    {
        LocalDownloadJob(p, ROOT_DIR() + "/no_such_file", "some_etag");
        FAIL();
    }
    catch (provider::NotExistsException const&)
    {
    }
}

TEST_F(LocalProviderTest, update)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    EXPECT_TRUE(job->isValid());

    auto file = job->item();
    auto old_etag = file.etag();

    int const segments = 50;
    unique_ptr<Uploader> uploader(file.createUploader(Item::ErrorIfConflict, file_contents.size() * segments));

    int count = 0;
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(10);
    QObject::connect(&timer, &QTimer::timeout, [&] {
            uploader->write(&file_contents[0], file_contents.size());
            count++;
            if (count == segments)
            {
                uploader->close();
            }
        });

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    timer.start();
    while (uploader->status() == Uploader::Loading ||
           uploader->status() == Uploader::Ready)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    ASSERT_EQ(Uploader::Finished, uploader->status()) << uploader->error().errorString().toStdString();

    file = uploader->item();
    EXPECT_NE(old_etag, file.etag());
    EXPECT_EQ(int64_t(file_contents.size() * segments), file.sizeInBytes());
}

TEST_F(LocalProviderTest, update_empty)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    EXPECT_TRUE(job->isValid());

    auto file = job->item();
    auto old_etag = file.etag();

    sleep(1);  // Make sure mtime changes.
    unique_ptr<Uploader> uploader(file.createUploader(Item::ErrorIfConflict, 0));
    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        while (uploader->status() != Uploader::Ready)
        {
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    uploader->close();
    while (uploader->status() != Uploader::Finished)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }

    file = uploader->item();
    EXPECT_NE(old_etag, file.etag());
    EXPECT_EQ(int64_t(0), file.sizeInBytes());
}

TEST_F(LocalProviderTest, update_cancel)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    EXPECT_TRUE(job->isValid());

    auto file = job->item();
    auto old_etag = file.etag();

    int const segments = 50;
    unique_ptr<Uploader> uploader(file.createUploader(Item::ErrorIfConflict, file_contents.size() * segments));

    int count = 0;
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(10);
    QObject::connect(&timer, &QTimer::timeout, [&] {
            uploader->write(&file_contents[0], file_contents.size());
            count++;
            if (count == segments / 2)
            {
                uploader->cancel();
            }
            else if (count == segments)
            {
                uploader->close();
            }
        });

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    timer.start();
    while (uploader->status() != Uploader::Cancelled)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
}

TEST_F(LocalProviderTest, update_file_touched_before_uploading)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    EXPECT_TRUE(job->isValid());

    auto file = job->item();
    auto old_etag = file.etag();

    sleep(1);  // Make sure mtime changes.
    cmd = string("touch ") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));
    unique_ptr<Uploader> uploader(file.createUploader(Item::ErrorIfConflict, 0));
    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    while (uploader->status() != Uploader::Error)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    EXPECT_EQ("update(): etag mismatch", uploader->error().message().toStdString());
}

TEST_F(LocalProviderTest, update_file_touched_while_uploading)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    EXPECT_TRUE(job->isValid());

    auto file = job->item();
    auto old_etag = file.etag();

    int const segments = 50;
    unique_ptr<Uploader> uploader(file.createUploader(Item::ErrorIfConflict, file_contents.size() * segments));

    int count = 0;
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(10);
    QObject::connect(&timer, &QTimer::timeout, [&] {
            uploader->write(&file_contents[0], file_contents.size());
            count++;
            if (count == segments / 2)
            {
                sleep(1);
                cmd = string("touch ") + full_path;
                ASSERT_EQ(0, system(cmd.c_str()));
            }
            else if (count == segments)
            {
                uploader->close();
            }
        });

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    timer.start();
    while (uploader->status() != Uploader::Error)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    ASSERT_EQ(Uploader::Error, uploader->status());
    EXPECT_EQ("update(): etag mismatch", uploader->error().message().toStdString());
}

TEST_F(LocalProviderTest, update_ignore_etag_mismatch)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    EXPECT_TRUE(job->isValid());

    auto file = job->item();
    auto old_etag = file.etag();

    sleep(1);  // Make sure mtime changes.
    cmd = string("touch ") + full_path;
    unique_ptr<Uploader> uploader(file.createUploader(Item::IgnoreConflict, 0));
    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        while (uploader->status() != Uploader::Ready)
        {
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    uploader->close();
    while (uploader->status() != Uploader::Finished)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }

    file = uploader->item();
    EXPECT_NE(old_etag, file.etag());
    EXPECT_EQ(int64_t(0), file.sizeInBytes());
}

TEST_F(LocalProviderTest, update_close_too_soon)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    EXPECT_TRUE(job->isValid());

    auto file = job->item();
    auto old_etag = file.etag();

    int const segments = 50;
    unique_ptr<Uploader> uploader(file.createUploader(Item::ErrorIfConflict, file_contents.size() * segments));

    int count = 0;
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(10);
    QObject::connect(&timer, &QTimer::timeout, [&] {
            uploader->write(&file_contents[0], file_contents.size());
            count++;
            if (count == segments - 1)
            {
                uploader->close();
            }
        });

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    timer.start();
    while (uploader->status() != Uploader::Error)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    ASSERT_EQ(Uploader::Error, uploader->status()) << uploader->error().errorString().toStdString();
    EXPECT_EQ("LogicError: finish() method called too early, size was given as 22300 but only 21854 bytes were received",
              uploader->error().errorString().toStdString());
}

TEST_F(LocalProviderTest, update_write_too_much)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    unique_ptr<ItemJob> job(acc_.get(QString::fromStdString(full_path)));
    wait(job.get());
    EXPECT_TRUE(job->isValid());

    auto file = job->item();
    auto old_etag = file.etag();

    int const segments = 50;
    // We write more than this many bytes below.
    unique_ptr<Uploader> uploader(file.createUploader(Item::ErrorIfConflict, file_contents.size() * segments - 1));

    int count = 0;
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(10);
    QObject::connect(&timer, &QTimer::timeout, [&] {
            uploader->write(&file_contents[0], file_contents.size());
            count++;
            if (count == segments)
            {
                uploader->close();
            }
        });

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    timer.start();
    while (uploader->status() != Uploader::Error)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    ASSERT_EQ(Uploader::Error, uploader->status());
    EXPECT_EQ("update(): received more than the expected number (22299) of bytes",
              uploader->error().message().toStdString());
}

TEST_F(LocalProviderTest, upload_wrong_file_type)
{
    // We can't try an upload for a directory via the client API, so we use the LocalUploadJob directly.

    auto p = make_shared<LocalProvider>();

    string const dir = ROOT_DIR() + "/dir";
    ASSERT_EQ(0, mkdir(dir.c_str(), 0755));

    try
    {
        LocalUploadJob(p, dir, 0, "");
        FAIL();
    }
    catch (provider::InvalidArgumentException const& e)
    {
        EXPECT_EQ(string("InvalidArgumentException: update(): \"" + dir + "\" is not a file"), e.what());
    }
}

TEST_F(LocalProviderTest, upload_root_noperm)
{
    // Force an error in prepare_channels when creating temp file.

    auto p = make_shared<LocalProvider>();

    ASSERT_EQ(0, chmod(ROOT_DIR().c_str(), 0644));

    try
    {
        LocalUploadJob(p, ROOT_DIR(), "name", 0, true);
        chmod(ROOT_DIR().c_str(), 0755);
        FAIL();
    }
    catch (provider::ResourceException const& e)
    {
        EXPECT_EQ(string("ResourceException: create_file(): cannot create temp file \"") + ROOT_DIR()
                         + "/.storage-framework-%%%%-%%%%-%%%%-%%%%\": Invalid argument",
                  e.what());
    }
    chmod(ROOT_DIR().c_str(), 0755);
}

TEST_F(LocalProviderTest, sanitize)
{
    // Force various errors in sanitize() for coverage.

    auto p = make_shared<LocalProvider>();

    try
    {
        LocalUploadJob(p, ROOT_DIR(), "a/b", 0, true);
        FAIL();
    }
    catch (provider::InvalidArgumentException const& e)
    {
        EXPECT_STREQ("InvalidArgumentException: create_file(): name \"a/b\" cannot contain a slash", e.what());
    }

    try
    {
        LocalUploadJob(p, ROOT_DIR(), "..", 0, true);
        FAIL();
    }
    catch (provider::InvalidArgumentException const& e)
    {
        EXPECT_STREQ("InvalidArgumentException: create_file(): invalid name: \"..\"", e.what());
    }

    try
    {
        LocalUploadJob(p, ROOT_DIR(), ".storage-framework", 0, true);
        FAIL();
    }
    catch (provider::InvalidArgumentException const& e)
    {
        EXPECT_STREQ("InvalidArgumentException: create_file(): names beginning with \".storage-framework\" are reserved",
                     e.what());
    }
}

TEST_F(LocalProviderTest, throw_if_not_valid)
{
    // Make sure that we can't escape the root.

    auto p = make_shared<LocalProvider>();

    try
    {
        LocalUploadJob(p, ROOT_DIR() + "/..", "a", 0, true);
        FAIL();
    }
    catch (provider::InvalidArgumentException const& e)
    {
        EXPECT_EQ(string("InvalidArgumentException: create_file(): invalid id: \"") + ROOT_DIR() + "/..\"", e.what());
    }

    try
    {
        LocalUploadJob(p, "/bin" , "a", 0, true);
        FAIL();
    }
    catch (provider::InvalidArgumentException const& e)
    {
        EXPECT_STREQ("InvalidArgumentException: create_file(): invalid id: \"/bin\"", e.what());
    }
}

TEST_F(LocalProviderTest, create_file)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto root = get_root(acc_);
    int const segments = 50;
    unique_ptr<Uploader> uploader(root.createFile("foo.txt", Item::ErrorIfConflict,
                                                  file_contents.size() * segments, "text/plain"));

    int count = 0;
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(10);
    QObject::connect(&timer, &QTimer::timeout, [&] {
            uploader->write(&file_contents[0], file_contents.size());
            count++;
            if (count == segments)
            {
                uploader->close();
            }
        });

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    while (uploader->status() != Uploader::Ready)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    timer.start();
    while (uploader->status() != Uploader::Finished)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }

    auto file = uploader->item();
    EXPECT_EQ(int64_t(file_contents.size() * segments), file.sizeInBytes());
}

TEST_F(LocalProviderTest, create_file_ignore_conflict)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    auto root = get_root(acc_);
    int const segments = 50;
    unique_ptr<Uploader> uploader(root.createFile("foo.txt", Item::IgnoreConflict,
                                                  file_contents.size() * segments, "text/plain"));

    int count = 0;
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(10);
    QObject::connect(&timer, &QTimer::timeout, [&] {
            uploader->write(&file_contents[0], file_contents.size());
            count++;
            if (count == segments)
            {
                uploader->close();
            }
        });

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    while (uploader->status() != Uploader::Ready)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    timer.start();
    while (uploader->status() != Uploader::Finished)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }

    auto file = uploader->item();
    EXPECT_EQ(int64_t(file_contents.size() * segments), file.sizeInBytes());
}

TEST_F(LocalProviderTest, create_file_error_if_conflict)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto full_path = ROOT_DIR() + "/foo.txt";
    auto cmd = string("echo hello >") + full_path;
    ASSERT_EQ(0, system(cmd.c_str()));

    auto root = get_root(acc_);
    int const segments = 50;
    unique_ptr<Uploader> uploader(root.createFile("foo.txt", Item::ErrorIfConflict,
                                                  file_contents.size() * segments, "text/plain"));

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    while (uploader->status() != Uploader::Error)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    EXPECT_EQ(string("create_file(): \"") + full_path + "\" exists already",
              uploader->error().message().toStdString());
}

TEST_F(LocalProviderTest, create_file_created_during_upload)
{
    using namespace unity::storage::qt;

    set_provider(unique_ptr<provider::ProviderBase>(new LocalProvider));

    auto root = get_root(acc_);
    int const segments = 50;
    string full_path = ROOT_DIR() + "/foo.txt";
    unique_ptr<Uploader> uploader(root.createFile("foo.txt", Item::ErrorIfConflict,
                                                  file_contents.size() * segments, "text/plain"));

    int count = 0;
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(10);
    QObject::connect(&timer, &QTimer::timeout, [&] {
            uploader->write(&file_contents[0], file_contents.size());
            count++;
            if (count == segments / 2)
            {
                string cmd = "touch " + full_path;
                ASSERT_EQ(0, system(cmd.c_str()));
            }
            else if (count == segments)
            {
                uploader->close();
            }
        });

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    while (uploader->status() != Uploader::Ready)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    timer.start();
    while (uploader->status() != Uploader::Error)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    EXPECT_EQ(string("create_file(): \"") + full_path + "\" exists already",
              uploader->error().message().toStdString());
}

int main(int argc, char** argv)
{
    setenv("LANG", "C", true);

    // Test test fixture repeatedly creates and tears down the dbus connection.
    // The provider calls g_file_new_for_path() which talks to the GVfs backend
    // via dbus. If the dbus connection disappears, that causes GIO to send a
    // a SIGTERM, killing the test.
    // Setting GIO_USE_VFS variable to "local" disables sending the signal.
    setenv("GIO_USE_VFS", "local", true);

    QCoreApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    int rc = RUN_ALL_TESTS();

    // Process any pending events to avoid bogus leak reports from valgrind.
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents();

    return rc;
}
