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

#include <unity/storage/qt/client-api.h>

#include "MockProvider.h"
#include <utils/gtest_printer.h>
#include <utils/ProviderFixture.h>

#include <gtest/gtest.h>
#include <QSignalSpy>

#include <unordered_set>

using namespace unity::storage;
using namespace unity::storage::qt;
using namespace std;

// Yes, that's ridiculously long, but the builders in Jenkins and the CI Train
// are stupifyingly slow at times.
static constexpr int SIGNAL_WAIT_TIME = 30000;

class RemoteClientTest : public ProviderFixture
{
protected:
    void SetUp() override
    {
        ProviderFixture::SetUp();
        runtime_.reset(new Runtime(connection()));
        acc_ = runtime_->make_test_account(bus_name(), object_path());
    }

    void TearDown() override
    {
        runtime_.reset();
        ProviderFixture::TearDown();
    }

    unique_ptr<Runtime> runtime_;
    Account acc_;
};

class RuntimeTest : public ProviderFixture {};

class AccountTest : public RemoteClientTest {};
class CopyTest : public RemoteClientTest {};
class CreateFileTest : public RemoteClientTest {};
class CreateFolderTest : public RemoteClientTest {};
class DeleteTest : public RemoteClientTest {};
class DownloadTest : public RemoteClientTest {};
class GetTest : public RemoteClientTest {};
class ItemTest : public RemoteClientTest {};
class ListTest : public RemoteClientTest {};
class LookupTest : public RemoteClientTest {};
class MetadataTest : public RemoteClientTest {};
class MoveTest : public RemoteClientTest {};
class ParentsTest : public RemoteClientTest {};
class RootsTest : public RemoteClientTest {};
class UploadTest : public RemoteClientTest {};

TEST(Runtime, lifecycle)
{
    Runtime runtime;
    EXPECT_TRUE(runtime.isValid());
    EXPECT_EQ(StorageError::Type::NoError, runtime.error().type());

    EXPECT_EQ(StorageError::Type::NoError, runtime.shutdown().type());
    EXPECT_FALSE(runtime.isValid());
    EXPECT_EQ(StorageError::Type::NoError, runtime.error().type());

    // Check that a second shutdown sets the error.
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, runtime.shutdown().type());
    EXPECT_FALSE(runtime.isValid());
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, runtime.error().type());
    EXPECT_EQ("Runtime::shutdown(): Runtime was destroyed previously", runtime.error().message());
}

#if 0
// TODO, how to test this?
TEST_F(RuntimeTest, init_error)
{
    QDBusConnection conn(connection());
    EXPECT_TRUE(conn.isConnected());
    dbus_.reset();  // Destroying the DBusEnvironment in the fixture forces disconnection.
    EXPECT_FALSE(conn.isConnected());

    Runtime rt(conn);
    EXPECT_FALSE(rt.isValid());
    EXPECT_FALSE(rt.connection().isConnected());
    auto e = rt.error();
    EXPECT_EQ(StorageError::Type::LocalCommsError, e.type());
    EXPECT_EQ("Runtime(): DBus connection is not connected", e.message());
}
#endif

TEST_F(AccountTest, basic)
{
    {
        // Default constructor.
        Account a;
        EXPECT_FALSE(a.isValid());
        EXPECT_EQ("", a.ownerId());
        EXPECT_EQ("", a.owner());
        EXPECT_EQ("", a.description());
    }

    {
        auto acc = runtime_->make_test_account(bus_name(), object_path(),
                                               "id", "owner", "description");
        EXPECT_TRUE(acc.isValid());
        EXPECT_EQ("id", acc.ownerId());
        EXPECT_EQ("owner", acc.owner());
        EXPECT_EQ("description", acc.description());

        // Copy constructor
        Account a2(acc);
        EXPECT_TRUE(a2.isValid());
        EXPECT_EQ("id", a2.ownerId());
        EXPECT_EQ("owner", a2.owner());
        EXPECT_EQ("description", a2.description());

        // Move constructor
        Account a3(move(a2));
        EXPECT_TRUE(a3.isValid());
        EXPECT_EQ("id", a3.ownerId());
        EXPECT_EQ("owner", a3.owner());
        EXPECT_EQ("description", a3.description());

        // Moved-from object must be invalid
        EXPECT_FALSE(a2.isValid());

        // Moved-from object must be assignable
        auto a4 = runtime_->make_test_account(bus_name(), object_path(),
                                              "id4", "owner4", "description4");
        a2 = a4;
        EXPECT_TRUE(a2.isValid());
        EXPECT_EQ("id4", a2.ownerId());
        EXPECT_EQ("owner4", a2.owner());
        EXPECT_EQ("description4", a2.description());
    }

    {
        auto a1 = runtime_->make_test_account(bus_name(), object_path(), "id", "owner", "description");
        auto a2 = runtime_->make_test_account(bus_name(), object_path(), "id2", "owner2", "description2");

        // Copy assignment
        a1 = a2;
        EXPECT_TRUE(a2.isValid());
        EXPECT_EQ("id2", a1.ownerId());
        EXPECT_EQ("owner2", a2.owner());
        EXPECT_EQ("description2", a1.description());

        // Self-assignment
        a2 = a2;
        EXPECT_TRUE(a2.isValid());
        EXPECT_EQ("id2", a1.ownerId());
        EXPECT_EQ("owner2", a2.owner());
        EXPECT_EQ("description2", a1.description());

        // Move assignment
        auto a3 = runtime_->make_test_account(bus_name(), object_path(),
                                              "id3", "owner3", "description3");
        a1 = move(a3);
        EXPECT_TRUE(a1.isValid());
        EXPECT_EQ("id3", a1.ownerId());
        EXPECT_EQ("owner3", a1.owner());
        EXPECT_EQ("description3", a1.description());

        // Moved-from object must be invalid
        EXPECT_FALSE(a3.isValid());

        // Moved-from object must be assignable
        auto a4 = runtime_->make_test_account(bus_name(), object_path(),
                                              "id4", "owner4", "description4");
        a2 = a4;
        EXPECT_TRUE(a2.isValid());
        EXPECT_EQ("id4", a2.ownerId());
        EXPECT_EQ("owner4", a2.owner());
        EXPECT_EQ("description4", a2.description());
    }
}

TEST_F(AccountTest, comparison)
{
    {
        // Both accounts invalid.
        Account a1;
        Account a2;
        EXPECT_TRUE(a1 == a2);
        EXPECT_FALSE(a1 != a2);
        EXPECT_FALSE(a1 < a2);
        EXPECT_TRUE(a1 <= a2);
        EXPECT_FALSE(a1 > a2);
        EXPECT_TRUE(a1 >= a2);
    }

    {
        // a1 valid, a2 invalid
        auto a1 = runtime_->make_test_account(bus_name(), object_path());
        Account a2;
        EXPECT_FALSE(a1 == a2);
        EXPECT_TRUE(a1 != a2);
        EXPECT_FALSE(a1 < a2);
        EXPECT_FALSE(a1 <= a2);
        EXPECT_TRUE(a1 > a2);
        EXPECT_TRUE(a1 >= a2);

        // And with swapped operands:
        EXPECT_FALSE(a2 == a1);
        EXPECT_TRUE(a2 != a1);
        EXPECT_TRUE(a2 < a1);
        EXPECT_TRUE(a2 <= a1);
        EXPECT_FALSE(a2 > a1);
        EXPECT_FALSE(a2 >= a1);
    }

    {
        // a1 < a2 for owner ID
        auto a1 = runtime_->make_test_account(bus_name(), object_path(), "a", "x", "x");
        auto a2 = runtime_->make_test_account(bus_name(), object_path(), "b", "x", "x");

        EXPECT_FALSE(a1 == a2);
        EXPECT_TRUE(a1 != a2);
        EXPECT_TRUE(a1 < a2);
        EXPECT_TRUE(a1 <= a2);
        EXPECT_FALSE(a1 > a2);
        EXPECT_FALSE(a1 >= a2);

        // And with swapped operands:
        EXPECT_FALSE(a2 == a1);
        EXPECT_TRUE(a2 != a1);
        EXPECT_FALSE(a2 < a1);
        EXPECT_FALSE(a2 <= a1);
        EXPECT_TRUE(a2 > a1);
        EXPECT_TRUE(a2 >= a1);
    }

    {
        // a1 < a2 for owner
        auto a1 = runtime_->make_test_account(bus_name(), object_path(), "a", "a", "x");
        auto a2 = runtime_->make_test_account(bus_name(), object_path(), "a", "b", "x");

        EXPECT_FALSE(a1 == a2);
        EXPECT_TRUE(a1 != a2);
        EXPECT_TRUE(a1 < a2);
        EXPECT_TRUE(a1 <= a2);
        EXPECT_FALSE(a1 > a2);
        EXPECT_FALSE(a1 >= a2);

        // And with swapped operands:
        EXPECT_FALSE(a2 == a1);
        EXPECT_TRUE(a2 != a1);
        EXPECT_FALSE(a2 < a1);
        EXPECT_FALSE(a2 <= a1);
        EXPECT_TRUE(a2 > a1);
        EXPECT_TRUE(a2 >= a1);
    }

    {
        // a1 < a2 for description
        auto a1 = runtime_->make_test_account(bus_name(), object_path(), "a", "a", "a");
        auto a2 = runtime_->make_test_account(bus_name(), object_path(), "a", "a", "b");

        EXPECT_FALSE(a1 == a2);
        EXPECT_TRUE(a1 != a2);
        EXPECT_TRUE(a1 < a2);
        EXPECT_TRUE(a1 <= a2);
        EXPECT_FALSE(a1 > a2);
        EXPECT_FALSE(a1 >= a2);

        // And with swapped operands:
        EXPECT_FALSE(a2 == a1);
        EXPECT_TRUE(a2 != a1);
        EXPECT_FALSE(a2 < a1);
        EXPECT_FALSE(a2 <= a1);
        EXPECT_TRUE(a2 > a1);
        EXPECT_TRUE(a2 >= a1);
    }

    {
        // a1 == a2
        auto a1 = runtime_->make_test_account(bus_name(), object_path(), "a", "a", "a");
        auto a2 = runtime_->make_test_account(bus_name(), object_path(), "a", "a", "a");

        EXPECT_TRUE(a1 == a2);
        EXPECT_FALSE(a1 != a2);
        EXPECT_FALSE(a1 < a2);
        EXPECT_TRUE(a1 <= a2);
        EXPECT_FALSE(a1 > a2);
        EXPECT_TRUE(a1 >= a2);

        // And with swapped operands:
        EXPECT_TRUE(a2 == a1);
        EXPECT_FALSE(a2 != a1);
        EXPECT_FALSE(a2 < a1);
        EXPECT_TRUE(a2 <= a1);
        EXPECT_FALSE(a2 > a1);
        EXPECT_TRUE(a2 >= a1);
    }
}

TEST_F(AccountTest, hash)
{
    unordered_set<Account>();  // Just to show that this works.

    Account a1;
    EXPECT_EQ(0, hash<Account>()(a1));
    EXPECT_EQ(0, a1.hash());
    EXPECT_EQ(0, qHash(a1));

    auto a2 = runtime_->make_test_account(bus_name(), object_path(), "a", "a", "a");
    // Due to different return types (size_t vs uint), hash() and qHash() do not return the same value.
    EXPECT_NE(0, a2.hash());
    EXPECT_NE(0, qHash(a2));
}

TEST_F(AccountTest, accounts)
{
    unique_ptr<AccountsJob> j(runtime_->accounts());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(AccountsJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());
    EXPECT_EQ(QList<Account>(), j->accounts());  // We haven't waited for the result yet.

    QSignalSpy spy(j.get(), &AccountsJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(AccountsJob::Status::Finished, qvariant_cast<AccountsJob::Status>(arg.at(0)));

    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(AccountsJob::Status::Finished, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    EXPECT_TRUE(runtime_->connection().isConnected());  // Just for coverage.

    auto accounts = j->accounts();

    // We don't check the contents of accounts here because we are using the real online accounts manager
    // in this test. This means that the number and kind of accounts that are returned depends
    // on what provider accounts the test user has configured.
}

TEST_F(AccountTest, runtime_destroyed)
{
    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime.

    AccountsJob* j = runtime_->accounts();
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(AccountsJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, j->error().type());
    EXPECT_EQ("Runtime::accounts(): Runtime was destroyed previously", j->error().message());
    EXPECT_EQ(QList<Account>(), j->accounts());

    // Signal must be received.
    QSignalSpy spy(j, &AccountsJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(AccountsJob::Status::Error, qvariant_cast<AccountsJob::Status>(arg.at(0)));
}

TEST_F(RootsTest, roots)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider));

    unique_ptr<ItemListJob> j(acc_.roots());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    // Check that we get the statusChanged and itemsReady signals.
    QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
    QSignalSpy status_spy(j.get(), &ItemListJob::statusChanged);

    ASSERT_TRUE(ready_spy.wait(SIGNAL_WAIT_TIME));

    ASSERT_EQ(1, ready_spy.count());
    auto arg = ready_spy.takeFirst();
    auto items = qvariant_cast<QList<Item>>(arg.at(0));
    ASSERT_EQ(1, items.size());

    ASSERT_EQ(1, status_spy.count());
    arg = status_spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Finished, qvariant_cast<ItemListJob::Status>(arg.at(0)));
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Finished, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    // Check contents of returned item.
    auto root = items[0];
    EXPECT_TRUE(root.isValid());
    EXPECT_EQ(Item::Type::Root, root.type());
    EXPECT_EQ("root_id", root.itemId());
    EXPECT_EQ("Root", root.name());
    EXPECT_EQ("etag", root.etag());
    EXPECT_EQ(QList<QString>(), root.parentIds());
    EXPECT_FALSE(root.lastModifiedTime().isValid());
    EXPECT_EQ(acc_, root.account());
}

TEST_F(RootsTest, runtime_destroyed)
{
    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime.

    unique_ptr<ItemListJob> j(acc_.roots());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, j->error().type());
    EXPECT_EQ("Account::roots(): Runtime was destroyed previously", j->error().message());

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));
}

TEST_F(RootsTest, runtime_destroyed_while_item_list_job_running)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("roots_slow")));

    unique_ptr<ItemListJob> j(acc_.roots());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime, provider still sleeping

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ("Account::roots(): Runtime was destroyed previously", j->error().message());
}

TEST_F(RootsTest, invalid_account)
{
    Account a;
    unique_ptr<ItemListJob> j(a.roots());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("Account::roots(): cannot create job from invalid account", j->error().message());

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ("Account::roots(): cannot create job from invalid account", j->error().message());
}

TEST_F(RootsTest, exception)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("roots_throw")));

    unique_ptr<ItemListJob> j(acc_.roots());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());
    EXPECT_EQ("No error", j->error().message());

    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ(ItemListJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::PermissionDenied, j->error().type());
    EXPECT_EQ("PermissionDenied: roots(): I'm sorry Dave, I'm afraid I can't do that.", j->error().errorString());
}

TEST_F(RootsTest, not_a_root)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("not_a_root")));

    unique_ptr<ItemListJob> j(acc_.roots());

    QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
    QSignalSpy status_spy(j.get(), &ItemListJob::statusChanged);
    status_spy.wait(SIGNAL_WAIT_TIME);
    auto arg = status_spy.takeFirst();

    // Bad metadata is ignored, so status is finished, and itemsReady was never called.
    EXPECT_EQ(ItemListJob::Status::Finished, qvariant_cast<ItemListJob::Status>(arg.at(0)));
    EXPECT_EQ(0, status_spy.count());
}

TEST_F(GetTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    // Get root.
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));

        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);

        EXPECT_EQ("root_id", j->item().itemId());
        EXPECT_EQ("Root", j->item().name());
        EXPECT_EQ(Item::Type::Root, j->item().type());
    }

    // Get a file.
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));

        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);

        EXPECT_EQ("child_id", j->item().itemId());
        EXPECT_EQ("Child", j->item().name());
        EXPECT_EQ(Item::Type::File, j->item().type());
    }

    // Get a folder.
    {
        unique_ptr<ItemJob> j(acc_.get("child_folder_id"));

        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);

        EXPECT_EQ("child_folder_id", j->item().itemId());
        EXPECT_EQ("Child_Folder", j->item().name());
        EXPECT_EQ(Item::Type::Folder, j->item().type());
    }
}

TEST_F(GetTest, runtime_destroyed)
{
    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime.

    unique_ptr<ItemJob> j(acc_.get("root_id"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, j->error().type());
    EXPECT_EQ("Account::get(): Runtime was destroyed previously", j->error().message());

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemJob::Status::Error, qvariant_cast<ItemJob::Status>(arg.at(0)));

    EXPECT_EQ("Account::get(): Runtime was destroyed previously", j->error().message());
}

TEST_F(GetTest, runtime_destroyed_while_item_job_running)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("slow_metadata")));

    unique_ptr<ItemJob> j(acc_.get("child_folder_id"));
    EXPECT_TRUE(j->isValid());

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime, provider still sleeping

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemJob::Status::Error, qvariant_cast<ItemJob::Status>(arg.at(0)));

    EXPECT_EQ("Account::get(): Runtime was destroyed previously", j->error().message());
}

TEST_F(GetTest, invalid_account)
{
    Account a;
    unique_ptr<ItemJob> j(a.get("child_Id"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("Account::get(): cannot create job from invalid account", j->error().message());

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemJob::Status::Error, qvariant_cast<ItemJob::Status>(arg.at(0)));

    EXPECT_EQ("Account::get(): cannot create job from invalid account", j->error().message());
}

TEST_F(GetTest, empty_id_from_provider)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("empty_id")));

    unique_ptr<ItemJob> j(acc_.get("child_folder_id"));
    EXPECT_TRUE(j->isValid());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemJob::Status::Error, qvariant_cast<ItemJob::Status>(arg.at(0)));

    EXPECT_EQ("Account::get(): received invalid metadata from provider: item_id cannot be empty", j->error().message());
}

TEST_F(GetTest, no_such_id)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    unique_ptr<ItemJob> j(acc_.get("no_such_id"));
    EXPECT_TRUE(j->isValid());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemJob::Status::Error, qvariant_cast<ItemJob::Status>(arg.at(0)));

    // TODO: This is missing the method name
    EXPECT_EQ("NotExists: metadata(): no such item: no_such_id", j->error().errorString());
    EXPECT_EQ("no_such_id", j->error().itemId());
}

TEST_F(MetadataTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    {
        Item i;
        EXPECT_EQ(0, i.metadata().size());
    }

    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));

        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);

        EXPECT_EQ(0, j->item().sizeInBytes());
        EXPECT_EQ(0, j->item().metadata().size());
    }

    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));

        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);

        EXPECT_EQ(10, j->item().sizeInBytes());
        EXPECT_EQ(2, j->item().metadata().size());
    }
}

TEST_F(MetadataTest, no_parents)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("no_parents")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: "
              "file or folder must have at least one parent ID", j->error().errorString());
}

TEST_F(MetadataTest, empty_parent)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("empty_parent")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: "
              "parent_id of file or folder cannot be empty", j->error().errorString());
}

TEST_F(MetadataTest, root_with_parent)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("root_with_parent")));

    unique_ptr<ItemJob> j(acc_.get("root_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: "
              "parent_ids of root must be empty", j->error().errorString());
}

TEST_F(MetadataTest, empty_name)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("empty_name")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: "
              "name cannot be empty", j->error().errorString());
}

TEST_F(MetadataTest, empty_etag)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("empty_etag")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: "
              "etag of a file cannot be empty", j->error().errorString());
}

TEST_F(MetadataTest, unknown_key)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("unknown_key")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    // We only emit a warning for unknown keys.
    EXPECT_EQ(ItemJob::Status::Finished, j->status());
}

TEST_F(MetadataTest, missing_size)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("missing_key")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: "
              "missing key \"size_in_bytes\" in metadata for \"child_id\"", j->error().errorString());
}

TEST_F(MetadataTest, wrong_type_for_time)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("wrong_type_for_time")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: last_modified_time: "
              "expected value of type QString, but received value of type qlonglong", j->error().errorString());
}

TEST_F(MetadataTest, bad_parse_for_time)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("bad_parse_for_time")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: last_modified_time: "
              "value \"xyz\" does not parse as ISO-8601 date", j->error().errorString());
}

TEST_F(MetadataTest, missing_timezone)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("missing_timezone")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: last_modified_time: "
              "value \"2007-04-05T14:30\" lacks a time zone specification", j->error().errorString());
}

TEST_F(MetadataTest, wrong_type_for_size)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("wrong_type_for_size")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: size_in_bytes: "
              "expected value of type qlonglong, but received value of type QString", j->error().errorString());
}

TEST_F(MetadataTest, negative_size)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("negative_size")));

    unique_ptr<ItemJob> j(acc_.get("child_id"));

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);

    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ("LocalCommsError: Account::get(): received invalid metadata from provider: size_in_bytes: "
              "expected value >= 0, but received -1", j->error().errorString());
}

TEST_F(DeleteTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider));

    Item item;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        item = j->item();
    }

    unique_ptr<VoidJob> j(item.deleteItem());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(VoidJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    EXPECT_EQ("child_id", item.itemId());

    QSignalSpy spy(j.get(), &VoidJob::statusChanged);
    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

    EXPECT_EQ(VoidJob::Status::Finished, j->status());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());
    EXPECT_EQ(VoidJob::Status::Finished, j->status());
}

TEST_F(DeleteTest, no_such_item)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("delete_no_such_item")));

    Item item;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        item = j->item();
    }

    unique_ptr<VoidJob> j(item.deleteItem());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(VoidJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    EXPECT_EQ("child_id", item.itemId());

    QSignalSpy spy(j.get(), &VoidJob::statusChanged);
    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

    EXPECT_EQ(VoidJob::Status::Error, j->status());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(StorageError::Type::NotExists, j->error().type());
    EXPECT_EQ("delete_item(): no such item: child_id", j->error().message());
}

TEST_F(DeleteTest, delete_root)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider));

    Item item;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        item = j->item();
    }

    unique_ptr<VoidJob> j(item.deleteItem());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(VoidJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());

    // Signal must be received.
    QSignalSpy spy(j.get(), &VoidJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    auto arg = spy.takeFirst();
    EXPECT_EQ(VoidJob::Status::Error, qvariant_cast<VoidJob::Status>(arg.at(0)));

    EXPECT_EQ("Item::deleteItem(): cannot delete root", j->error().message());
}

TEST_F(DeleteTest, runtime_destroyed)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider));

    Item item;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        item = j->item();
    }

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime.

    unique_ptr<VoidJob> j(item.deleteItem());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(VoidJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, j->error().type());
    EXPECT_EQ("Item::deleteItem(): Runtime was destroyed previously", j->error().message());

    // Signal must be received.
    QSignalSpy spy(j.get(), &VoidJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    auto arg = spy.takeFirst();
    EXPECT_EQ(VoidJob::Status::Error, qvariant_cast<VoidJob::Status>(arg.at(0)));

    EXPECT_EQ("Item::deleteItem(): Runtime was destroyed previously", j->error().message());
}

TEST_F(DeleteTest, runtime_destroyed_while_void_job_running)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("slow_delete")));

    Item item;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        item = j->item();
    }

    unique_ptr<VoidJob> j(item.deleteItem());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(VoidJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime.

    // Signal must be received.
    QSignalSpy spy(j.get(), &VoidJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    auto arg = spy.takeFirst();
    EXPECT_EQ(VoidJob::Status::Error, qvariant_cast<VoidJob::Status>(arg.at(0)));

    EXPECT_EQ("Item::deleteItem(): Runtime was destroyed previously", j->error().message());
}

TEST_F(DeleteTest, invalid_item)
{
    Item i;
    unique_ptr<VoidJob> j(i.deleteItem());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(VoidJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("Item::deleteItem(): cannot create job from invalid item", j->error().message());

    // Signal must be received.
    QSignalSpy spy(j.get(), &VoidJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(VoidJob::Status::Error, qvariant_cast<VoidJob::Status>(arg.at(0)));

    EXPECT_EQ("Item::deleteItem(): cannot create job from invalid item", j->error().message());
}

#if 0
// TODO: need to make internal symbols available for testing.
TEST_F(ValidateTest, basic)
{
    using namespace unity::storage::qt::internal;

    unity::storage::internal::ItemMetadata md;

    validate("foo", md);
}
#endif

TEST_F(ItemTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    {
        // Default constructor.
        Item i;
        EXPECT_FALSE(i.isValid());
        EXPECT_EQ("", i.itemId());
        EXPECT_EQ("", i.name());
        EXPECT_EQ("", i.etag());
        EXPECT_EQ(Item::Type::File, i.type());
        EXPECT_EQ(0, i.metadata().size());
        EXPECT_EQ(0, i.sizeInBytes());
        auto mtime = i.lastModifiedTime();
        EXPECT_FALSE(mtime.isValid());
        auto pids = i.parentIds();
        EXPECT_EQ(0, pids.size());
    }

    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));

        {
            QSignalSpy spy(j.get(), &ItemJob::statusChanged);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        Item i = j->item();
        EXPECT_TRUE(i.isValid());
        EXPECT_EQ("child_id", i.itemId());
        EXPECT_EQ("Child", i.name());
        EXPECT_EQ(10, i.sizeInBytes());
        EXPECT_TRUE(i.account().isValid());
        EXPECT_EQ("etag", i.etag());
        EXPECT_EQ(Item::Type::File, i.type());

        // Copy constructor
        Item i2(i);
        EXPECT_EQ(i, i2);

        // Move constructor
        Item i3(move(i2));
        EXPECT_TRUE(i3.isValid());
        EXPECT_EQ(i, i3);

        // Moved-from object must be invalid
        EXPECT_FALSE(i2.isValid());

        // Moved-from object must be assignable
        j.reset(acc_.get("child_id"));
        {
            QSignalSpy spy(j.get(), &ItemJob::statusChanged);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto i4 = j->item();
        i2 = i4;
        EXPECT_EQ(i4, i2);
    }

    {
        unique_ptr<ItemJob> j1(acc_.get("child_id"));
        unique_ptr<ItemJob> j2(acc_.get("root_id"));

        QSignalSpy spy1(j1.get(), &ItemJob::statusChanged);
        QSignalSpy spy2(j2.get(), &ItemJob::statusChanged);
        spy2.wait(SIGNAL_WAIT_TIME);
        ASSERT_EQ(1, spy1.count());

        auto i1 = j1->item();
        auto i2 = j2->item();

        // Copy assignment
        i1 = i2;
        EXPECT_TRUE(i2.isValid());
        EXPECT_EQ(i2, i1);

        // Self-assignment
        i2 = i2;
        EXPECT_TRUE(i2.isValid());
        EXPECT_EQ("root_id", i2.itemId());
        EXPECT_EQ("Root", i2.name());
        EXPECT_TRUE(i2.account().isValid());
        EXPECT_EQ("etag", i2.etag());
        EXPECT_EQ(Item::Type::Root, i2.type());

        // Move assignment
        unique_ptr<ItemJob> j3(acc_.get("child_folder_id"));
        QSignalSpy spy(j3.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        ASSERT_EQ(1, spy1.count());

        auto i3 = j3->item();

        i1 = move(i3);
        EXPECT_TRUE(i1.isValid());
        EXPECT_EQ("child_folder_id", i1.itemId());
        EXPECT_EQ("Child_Folder", i1.name());
        EXPECT_EQ(i1.account(), i1.account());
        EXPECT_EQ("etag", i1.etag());
        EXPECT_EQ(Item::Type::Folder, i1.type());

        // Moved-from object must be invalid
        EXPECT_FALSE(i3.isValid());

        // Moved-from object must be assignable
        i3 = i2;
        EXPECT_EQ(i2, i3);
    }
}

TEST_F(ItemTest, comparison_and_hash)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider));

    {
        // Both items invalid.
        Item i1;
        Item i2;
        EXPECT_TRUE(i1 == i2);
        EXPECT_FALSE(i1 != i2);
        EXPECT_FALSE(i1 < i2);
        EXPECT_TRUE(i1 <= i2);
        EXPECT_FALSE(i1 > i2);
        EXPECT_TRUE(i1 >= i2);

        unordered_set<Item>();  // Just to show that this works.

        EXPECT_EQ(0, hash<Item>()(i1));
        EXPECT_EQ(0, i1.hash());
        EXPECT_EQ(0, qHash(i1));
    }

    {
        // i1 valid, i2 invalid
        unique_ptr<ItemListJob> j(acc_.roots());

        QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
        ASSERT_TRUE(ready_spy.wait(SIGNAL_WAIT_TIME));

        ASSERT_EQ(1, ready_spy.count());
        auto arg = ready_spy.takeFirst();
        auto items = qvariant_cast<QList<Item>>(arg.at(0));
        ASSERT_EQ(1, items.size());

        auto i1 = items[0];
        Item i2;
        EXPECT_FALSE(i1 == i2);
        EXPECT_TRUE(i1 != i2);
        EXPECT_FALSE(i1 < i2);
        EXPECT_FALSE(i1 <= i2);
        EXPECT_TRUE(i1 > i2);
        EXPECT_TRUE(i1 >= i2);

        // And with swapped operands:
        EXPECT_FALSE(i2 == i1);
        EXPECT_TRUE(i2 != i1);
        EXPECT_TRUE(i2 < i1);
        EXPECT_TRUE(i2 <= i1);
        EXPECT_FALSE(i2 > i1);
        EXPECT_FALSE(i2 >= i1);

        EXPECT_NE(0, i1.hash());
        EXPECT_NE(0, qHash(i1));
    }

    {
        // Both items valid with identical metadata, but different accounts (a1 < a2).

        auto a1 = runtime_->make_test_account(bus_name(), object_path(), "a", "x", "x");
        auto a2 = runtime_->make_test_account(bus_name(), object_path(), "b", "x", "x");

        Item i1;
        Item i2;

        {
            unique_ptr<ItemJob> j(a1.get("root_id"));
            QSignalSpy spy(j.get(), &ItemJob::statusChanged);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
            i1 = j->item();
        }
        {
            unique_ptr<ItemJob> j(a2.get("root_id"));
            QSignalSpy spy(j.get(), &ItemJob::statusChanged);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
            i2 = j->item();
        }

        ASSERT_EQ(i1.itemId(), i2.itemId());

        EXPECT_FALSE(i1 == i2);
        EXPECT_TRUE(i1 != i2);
        EXPECT_TRUE(i1 < i2);
        EXPECT_TRUE(i1 <= i2);
        EXPECT_FALSE(i1 > i2);
        EXPECT_FALSE(i1 >= i2);

        // And with swapped operands:
        EXPECT_FALSE(i2 == i1);
        EXPECT_TRUE(i2 != i1);
        EXPECT_FALSE(i2 < i1);
        EXPECT_FALSE(i2 <= i1);
        EXPECT_TRUE(i2 > i1);
        EXPECT_TRUE(i2 >= i1);
    }

    {
        // Both items valid with identical metadata, but different instances, so we do deep comparison.

        Item i1;
        Item i2;

        {
            unique_ptr<ItemJob> j(acc_.get("root_id"));
            QSignalSpy spy(j.get(), &ItemJob::statusChanged);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
            i1 = j->item();
        }
        {
            unique_ptr<ItemJob> j(acc_.get("root_id"));
            QSignalSpy spy(j.get(), &ItemJob::statusChanged);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
            i2 = j->item();
        }

        EXPECT_TRUE(i1 == i2);
        EXPECT_FALSE(i1 != i2);
        EXPECT_FALSE(i1 < i2);
        EXPECT_TRUE(i1 <= i2);
        EXPECT_FALSE(i1 > i2);
        EXPECT_TRUE(i1 >= i2);

        // And with swapped operands:
        EXPECT_TRUE(i2 == i1);
        EXPECT_FALSE(i2 != i1);
        EXPECT_FALSE(i2 < i1);
        EXPECT_TRUE(i2 <= i1);
        EXPECT_FALSE(i2 > i1);
        EXPECT_TRUE(i2 >= i1);

        EXPECT_EQ(i1.hash(), i2.hash());
        EXPECT_EQ(qHash(i1), qHash(i2));
    }
}

TEST_F(ParentsTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    {
        // Getting parents from root does not call the provider and returns
        // no parents immediately.
        unique_ptr<ItemListJob> j(root.parents());
        EXPECT_TRUE(j->isValid());
        EXPECT_EQ(ItemListJob::Status::Finished, j->status());
        EXPECT_EQ(StorageError::Type::NoError, j->error().type());

        // Signal must be received.
        QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        ASSERT_EQ(1, spy.count());
        auto arg = spy.takeFirst();
        EXPECT_EQ(ItemListJob::Status::Finished, qvariant_cast<ItemListJob::Status>(arg.at(0)));
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    QList<Item> parents;
    {
        unique_ptr<ItemListJob> j(child.parents());
        EXPECT_TRUE(j->isValid());
        EXPECT_EQ(ItemListJob::Status::Loading, j->status());
        EXPECT_EQ(StorageError::Type::NoError, j->error().type());

        QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
        QSignalSpy status_spy(j.get(), &ItemListJob::statusChanged);
        ready_spy.wait(SIGNAL_WAIT_TIME);
        ASSERT_EQ(1, ready_spy.count());
        auto list_arg = ready_spy.takeFirst();
        parents = qvariant_cast<QList<Item>>(list_arg.at(0));

        // When the signal for the final item arrives, status must be Finished.
        EXPECT_EQ(ItemListJob::Status::Finished, j->status());

        // Finished signal must be received.
        if (status_spy.count() == 0)
        {
            status_spy.wait(SIGNAL_WAIT_TIME);
        }
        ASSERT_EQ(1, status_spy.count());
        auto status_arg = status_spy.takeFirst();
        EXPECT_EQ(ItemListJob::Status::Finished, qvariant_cast<ItemListJob::Status>(status_arg.at(0)));

        // Child must have one parent, namely the root.
        ASSERT_EQ(1, parents.size());
        EXPECT_EQ(root, parents[0]);
    }
}

TEST_F(ParentsTest, two_parents)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("two_parents")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    QList<Item> parents;
    unique_ptr<ItemListJob> j(child.parents());
    EXPECT_TRUE(j->isValid());

    QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
    QSignalSpy status_spy(j.get(), &ItemListJob::statusChanged);

    ready_spy.wait(SIGNAL_WAIT_TIME);
    auto list_arg = ready_spy.takeFirst();
    auto this_parent = qvariant_cast<QList<Item>>(list_arg.at(0));
    parents.append(this_parent);
    while (ready_spy.count() < 1)
    {
        ready_spy.wait(SIGNAL_WAIT_TIME);
    }
    list_arg = ready_spy.takeFirst();
    this_parent = qvariant_cast<QList<Item>>(list_arg.at(0));
    parents.append(this_parent);

    // Finished signal must be received.
    if (status_spy.count() == 0)
    {
        status_spy.wait(SIGNAL_WAIT_TIME);
    }
    ASSERT_EQ(1, status_spy.count());
    auto status_arg = status_spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Finished, qvariant_cast<ItemListJob::Status>(status_arg.at(0)));

    // Child must have two parents.
    ASSERT_EQ(2, parents.size());
    EXPECT_EQ("root_id", parents[0].itemId());
    EXPECT_EQ("child_folder_id", parents[1].itemId());
}

TEST_F(ParentsTest, two_parents_throw)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("two_parents_throw")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    QList<Item> parents;
    {
        unique_ptr<ItemListJob> j(child.parents());
        EXPECT_TRUE(j->isValid());

        QSignalSpy status_spy(j.get(), &ItemListJob::statusChanged);
        QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);

        status_spy.wait(SIGNAL_WAIT_TIME);
        ASSERT_EQ(1, status_spy.count());
        auto status_arg = status_spy.takeFirst();
        EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(status_arg.at(0)));
        EXPECT_EQ(StorageError::Type::ResourceError, j->error().type());
        EXPECT_EQ("ResourceError: metadata(): weird error", j->error().errorString());
        EXPECT_EQ(42, j->error().errorCode());

        // We wait here to allow the error return for the second parent to arrive in MultiItemJobImpl.
        // This gives us coverage on the early return in the process_error lambda, when the job is
        // already in the error state.
        EXPECT_FALSE(ready_spy.wait(1000));
    }
}

TEST_F(ParentsTest, invalid_item)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item invalid;
    unique_ptr<ItemListJob> j(invalid.parents());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("Item::parents(): cannot create job from invalid item", j->error().message());

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));
}

TEST_F(ParentsTest, runtime_destroyed)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime.

    unique_ptr<ItemListJob> j(root.parents());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, j->error().type());
    EXPECT_EQ("Item::parents(): Runtime was destroyed previously", j->error().message());

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ("Item::parents(): Runtime was destroyed previously", j->error().message());
}

TEST_F(ParentsTest, runtime_destroyed_while_item_list_job_running)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("slow_metadata")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemListJob> j(child.parents());

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime, provider still sleeping

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ("Item::parents(): Runtime was destroyed previously", j->error().message());
}

TEST_F(ParentsTest, bad_metadata)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("bad_parent_metadata_from_child")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    {
        unique_ptr<ItemListJob> j(child.parents());
        EXPECT_TRUE(j->isValid());
        EXPECT_EQ(ItemListJob::Status::Loading, j->status());
        EXPECT_EQ(StorageError::Type::NoError, j->error().type());

        QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        ASSERT_EQ(1, spy.count());
        auto arg = spy.takeFirst();
        EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

        EXPECT_EQ("Item::parents(): provider returned a file as a parent", j->error().message());
    }
}

TEST_F(CopyTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemJob> j(child.copy(root, "copied_item"));
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Finished, status);

    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Finished, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());
    auto copied_file = j->item();
    EXPECT_EQ("new_item_id", copied_file.itemId());
    EXPECT_EQ(root.itemId(), copied_file.parentIds()[0]);
    EXPECT_EQ("copied_item", copied_file.name());
}

TEST_F(CopyTest, invalid)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    unique_ptr<ItemJob> j(child.copy(root, "copied_item"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError", j->error().name());
    EXPECT_EQ("LogicError: Item::copy(): cannot create job from invalid item", j->error().errorString());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError", j->error().name());
    EXPECT_EQ("LogicError: Item::copy(): cannot create job from invalid item", j->error().errorString());
}

TEST_F(CopyTest, invalid_parent)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    Item invalid_parent;
    unique_ptr<ItemJob> j(child.copy(invalid_parent, "copied_item"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::InvalidArgument, j->error().type());
    EXPECT_EQ("InvalidArgument: Item::copy(): newParent is invalid", j->error().errorString());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::InvalidArgument, j->error().type());
    EXPECT_EQ("InvalidArgument: Item::copy(): newParent is invalid", j->error().errorString());
}

TEST_F(CopyTest, empty_name)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemJob> j(child.copy(root, ""));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::InvalidArgument, j->error().type());
    EXPECT_EQ("InvalidArgument: Item::copy(): newName cannot be empty", j->error().errorString());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::InvalidArgument, j->error().type());
    EXPECT_EQ("InvalidArgument: Item::copy(): newName cannot be empty", j->error().errorString());
}

TEST_F(CopyTest, wrong_account)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    auto test_account = runtime_->make_test_account(bus_name(), object_path(), "test_account");

    Item root1;
    {
        unique_ptr<ItemJob> j(test_account.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root1 = j->item();
        EXPECT_TRUE(root1.isValid());
    }

    Item root2;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root2 = j->item();
        EXPECT_TRUE(root2.isValid());
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
        EXPECT_TRUE(child.isValid());
    }

    unique_ptr<ItemJob> j(child.copy(root1, "copied_Item"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::copy(): source and target must belong to the same account", j->error().errorString());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::copy(): source and target must belong to the same account", j->error().errorString());
}

TEST_F(CopyTest, wrong_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemJob> j(child.copy(child, "copied_Item"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::copy(): newParent cannot be a file", j->error().errorString());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::copy(): newParent cannot be a file", j->error().errorString());
}

TEST_F(CopyTest, type_mismatch)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("copy_type_mismatch")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemJob> j(child.copy(root, "copied_Item"));
    EXPECT_TRUE(j->isValid());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LocalCommsError, j->error().type());
    EXPECT_EQ("LocalCommsError: Item::copy(): source and target item type differ", j->error().errorString());
}

TEST_F(MoveTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemJob> j(child.move(root, "moved_item"));
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Finished, status);

    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Finished, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());
    auto moved_file = j->item();
    EXPECT_EQ("child_id", moved_file.itemId());
    EXPECT_EQ(root.itemId(), moved_file.parentIds()[0]);
    EXPECT_EQ("moved_item", moved_file.name());
}

TEST_F(MoveTest, invalid)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    unique_ptr<ItemJob> j(child.move(root, "moved_item"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError", j->error().name());
    EXPECT_EQ("LogicError: Item::move(): cannot create job from invalid item", j->error().errorString());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError", j->error().name());
    EXPECT_EQ("LogicError: Item::move(): cannot create job from invalid item", j->error().errorString());
}

TEST_F(MoveTest, root_returned)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("move_returns_root")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemJob> j(child.move(root, "moved_item"));
    EXPECT_TRUE(j->isValid());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LocalCommsError, j->error().type());
    EXPECT_EQ("LocalCommsError", j->error().name());
    EXPECT_EQ("LocalCommsError: Item::move(): impossible root item returned by provider", j->error().errorString());
}

TEST_F(MoveTest, type_mismatch)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("move_type_mismatch")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemJob> j(child.move(root, "moved_Item"));
    EXPECT_TRUE(j->isValid());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LocalCommsError, j->error().type());
    EXPECT_EQ("LocalCommsError: Item::move(): provider error: source and target item type differ",
              j->error().errorString());
}

TEST_F(LookupTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemListJob> j(root.lookup("Child"));
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
    QSignalSpy status_spy(j.get(), &ItemListJob::statusChanged);

    status_spy.wait(SIGNAL_WAIT_TIME);

    auto list_arg = ready_spy.takeFirst();
    auto list = qvariant_cast<QList<Item>>(list_arg.at(0));
    ASSERT_EQ(1, list.size());
    auto child = list[0];
    ASSERT_EQ("child_id", child.itemId());
    ASSERT_EQ("Child", child.name());

    auto arg = status_spy.takeFirst();
    auto status = qvariant_cast<ItemListJob::Status>(arg.at(0));
    EXPECT_EQ(ItemListJob::Status::Finished, status);

    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Finished, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());
}

TEST_F(LookupTest, invalid)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    unique_ptr<ItemListJob> j(root.lookup("Child"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::lookup(): cannot create job from invalid item", j->error().errorString());

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ("LogicError: Item::lookup(): cannot create job from invalid item", j->error().errorString());
}

TEST_F(LookupTest, wrong_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemListJob> j(child.lookup("Child"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::lookup(): cannot perform lookup on a file", j->error().errorString());

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ("LogicError: Item::lookup(): cannot perform lookup on a file", j->error().errorString());
}

TEST_F(CreateFolderTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemJob> j(root.createFolder("new_folder"));
    EXPECT_TRUE(j->isValid());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Finished, status);

    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Finished, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    auto new_folder = j->item();
    EXPECT_EQ("new_folder", new_folder.name());
    EXPECT_EQ(Item::Type::Folder, new_folder.type());
}

TEST_F(CreateFolderTest, invalid)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    unique_ptr<ItemJob> j(root.createFolder("new_folder"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::createFolder(): cannot create job from invalid item", j->error().errorString());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_EQ("LogicError: Item::createFolder(): cannot create job from invalid item", j->error().errorString());
}

TEST_F(CreateFolderTest, wrong_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemJob> j(child.createFolder("new_folder"));
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::createFolder(): cannot create a folder with a file as the parent",
              j->error().errorString());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_EQ("LogicError: Item::createFolder(): cannot create a folder with a file as the parent",
              j->error().errorString());
}

TEST_F(CreateFolderTest, wrong_return_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("create_folder_returns_file")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemJob> j(root.createFolder("new_folder"));
    EXPECT_TRUE(j->isValid());

    QSignalSpy spy(j.get(), &ItemJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemJob::Status>(arg.at(0));
    EXPECT_EQ(ItemJob::Status::Error, status);

    EXPECT_EQ("LocalCommsError: Item::createFolder(): impossible file item returned by provider",
              j->error().errorString());
}

TEST_F(ListTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemListJob> j(root.list());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
    QSignalSpy status_spy(j.get(), &ItemListJob::statusChanged);
    ready_spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, ready_spy.count());
    auto list_arg = ready_spy.takeFirst();
    auto items = qvariant_cast<QList<Item>>(list_arg.at(0));
    ASSERT_EQ(1, items.size());
    EXPECT_EQ("child_id", items[0].itemId());

    // When the signal for the final item arrives, status must be Finished.
    EXPECT_EQ(ItemListJob::Status::Finished, j->status());

    // Finished signal must be received.
    if (status_spy.count() == 0)
    {
        status_spy.wait(SIGNAL_WAIT_TIME);
    }
    ASSERT_EQ(1, status_spy.count());
    auto status_arg = status_spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Finished, qvariant_cast<ItemListJob::Status>(status_arg.at(0)));
}

TEST_F(ListTest, empty_list)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("list_empty")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemListJob> j(root.list());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
    QSignalSpy status_spy(j.get(), &ItemListJob::statusChanged);
    status_spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(0, ready_spy.count());

    EXPECT_EQ(ItemListJob::Status::Finished, j->status());

    ASSERT_EQ(1, status_spy.count());
    auto status_arg = status_spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Finished, qvariant_cast<ItemListJob::Status>(status_arg.at(0)));
}

TEST_F(ListTest, two_children)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("list_two_children")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemListJob> j(root.list());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    QSignalSpy ready_spy(j.get(), &ItemListJob::itemsReady);
    QSignalSpy status_spy(j.get(), &ItemListJob::statusChanged);

    QList<Item> items;
    ready_spy.wait(SIGNAL_WAIT_TIME);
    auto list_arg = ready_spy.takeFirst();
    auto this_item = qvariant_cast<QList<Item>>(list_arg.at(0));
    items.append(this_item);
    if (ready_spy.count() < 1)
    {
        ASSERT_TRUE(ready_spy.wait(SIGNAL_WAIT_TIME));
    }
    list_arg = ready_spy.takeFirst();
    this_item = qvariant_cast<QList<Item>>(list_arg.at(0));
    items.append(this_item);

    // Finished signal must be received.
    if (status_spy.count() == 0)
    {
        status_spy.wait(SIGNAL_WAIT_TIME);
    }
    ASSERT_EQ(1, status_spy.count());
    auto status_arg = status_spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Finished, qvariant_cast<ItemListJob::Status>(status_arg.at(0)));

    // Must have two children.
    ASSERT_EQ(2, items.size());
    EXPECT_EQ("child_id", items[0].itemId());
    EXPECT_EQ("child2_id", items[1].itemId());
}

TEST_F(ListTest, job_out_of_scope)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("list_slow")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemListJob> j(root.list());
    // Just to show that it's safe to drop the job on the floor while
    // operation is in progress.
}

TEST_F(ListTest, invalid)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    unique_ptr<ItemListJob> j(root.list());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::list(): cannot create job from invalid item", j->error().errorString());

    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemListJob::Status>(arg.at(0));
    EXPECT_EQ(ItemListJob::Status::Error, status);

    EXPECT_EQ("LogicError: Item::list(): cannot create job from invalid item", j->error().errorString());
}

TEST_F(ListTest, wrong_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<ItemListJob> j(child.list());
    EXPECT_FALSE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Error, j->status());
    EXPECT_EQ(StorageError::Type::LogicError, j->error().type());
    EXPECT_EQ("LogicError: Item::list(): cannot perform list on a file", j->error().errorString());

    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    auto status = qvariant_cast<ItemListJob::Status>(arg.at(0));
    EXPECT_EQ(ItemListJob::Status::Error, status);

    EXPECT_EQ("LogicError: Item::list(): cannot perform list on a file", j->error().errorString());
}

TEST_F(ListTest, wrong_return_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("list_return_root")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemListJob> j(root.list());
    EXPECT_TRUE(j->isValid());
    EXPECT_EQ(ItemListJob::Status::Loading, j->status());
    EXPECT_EQ(StorageError::Type::NoError, j->error().type());

    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ("LocalCommsError: Item::list(): impossible root item returned by provider", j->error().errorString());
}

TEST_F(ListTest, runtime_destroyed_while_item_list_job_running)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("list_slow")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemListJob> j(root.list());

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime, provider still sleeping

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ("Item::list(): Runtime was destroyed previously", j->error().message());
}

TEST_F(ListTest, no_permission)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("list_no_permission")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<ItemListJob> j(root.list());
    EXPECT_TRUE(j->isValid());

    // Signal must be received.
    QSignalSpy spy(j.get(), &ItemListJob::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(ItemListJob::Status::Error, qvariant_cast<ItemListJob::Status>(arg.at(0)));

    EXPECT_EQ("permission denied", j->error().message());
    EXPECT_EQ(StorageError::Type::PermissionDenied, j->error().type());
}

TEST_F(DownloadTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());
    EXPECT_EQ(Downloader::Status::Loading, downloader->status());
    EXPECT_EQ(StorageError::NoError, downloader->error().type());
    EXPECT_EQ(child, downloader->item());

    QSignalSpy status_spy(downloader.get(), &Downloader::statusChanged);
    {
        QSignalSpy read_spy(downloader.get(), &Downloader::readyRead);
        ASSERT_TRUE(status_spy.wait(SIGNAL_WAIT_TIME));
        auto arg = status_spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Ready, qvariant_cast<Downloader::Status>(arg.at(0)));

        if (read_spy.count() != 1)
        {
            read_spy.wait(SIGNAL_WAIT_TIME);
        }
    }
    auto data = downloader->readAll();
    EXPECT_EQ(QByteArray("Hello world", -1), data);

    downloader->finishDownload();
    ASSERT_TRUE(status_spy.wait(SIGNAL_WAIT_TIME));
    auto arg = status_spy.takeFirst();
    EXPECT_EQ(Downloader::Status::Finished, qvariant_cast<Downloader::Status>(arg.at(0)));
}

// TODO: This leaks:
// ==4645== 1,369 (272 direct, 1,097 indirect) bytes in 1 blocks are definitely lost in loss record 193 of 203
// ==4645==    at 0x4C2E0EF: operator new(unsigned long) (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
// ==4645==    by 0x4FD9D0A: boost::promise<void>::promise() (future.hpp:2309)
// ==4645==    by 0x4FDA6F2: boost::make_ready_future() (future.hpp:3935)
// ==4645==    by 0x4FD96B0: unity::storage::provider::internal::DownloadJobImpl::cancel(unity::storage::provider::DownloadJob&) (DownloadJobImpl.cpp:161)
// ==4645==    by 0x4FEE198: void unity::storage::provider::internal::PendingJobs::cancel_job<unity::storage::provider::DownloadJob>(std::shared_ptr<unity::storage::provider::DownloadJob> const&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) (PendingJobs.cpp:181)
// ==4645==    by 0x4FEA3BF: unity::storage::provider::internal::PendingJobs::~PendingJobs() (PendingJobs.cpp:55)
// ==4645==    by 0x4FEA6CD: unity::storage::provider::internal::PendingJobs::~PendingJobs() (PendingJobs.cpp:61)
// ==4645==    by 0x4F8E927: std::default_delete<unity::storage::provider::internal::PendingJobs>::operator()(unity::storage::provider::internal::PendingJobs*) const (unique_ptr.h:76)
// ==4645==    by 0x4F8D85B: std::unique_ptr<unity::storage::provider::internal::PendingJobs, std::default_delete<unity::storage::provider::internal::PendingJobs> >::~unique_ptr() (unique_ptr.h:236)
// ==4645==    by 0x4F89449: unity::storage::provider::internal::AccountData::~AccountData() (AccountData.h:51)
// ==4645==    by 0x504D9A6: void __gnu_cxx::new_allocator<unity::storage::provider::internal::AccountData>::destroy<unity::storage::provider::internal::AccountData>(unity::storage::provider::internal::AccountData*) (new_allocator.h:124)
// ==4645==    by 0x504D8AA: void std::allocator_traits<std::allocator<unity::storage::provider::internal::AccountData> >::destroy<unity::storage::provider::internal::AccountData>(std::allocator<unity::storage::provider::internal::AccountData>&, unity::storage::provider::internal::AccountData*) (alloc_traits.h:542)

TEST_F(DownloadTest, abandoned)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Ready, qvariant_cast<Downloader::Status>(arg.at(0)));
    }
}

TEST_F(DownloadTest, runtime_destroyed)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_FALSE(downloader->isValid());
    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::RuntimeDestroyed, downloader->error().type());
    EXPECT_EQ("RuntimeDestroyed: Item::createDownloader(): Runtime was destroyed previously",
              downloader->error().errorString());
    EXPECT_EQ(Item(), downloader->item());

    // Signal must arrive.
    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));
    }
}

TEST_F(DownloadTest, runtime_destroyed_while_download_running)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("download_slow")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime, provider still sleeping

    // Signal must arrive.
    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));
    }

    EXPECT_FALSE(downloader->isValid());
    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::RuntimeDestroyed, downloader->error().type());
    EXPECT_EQ("RuntimeDestroyed: Item::createDownloader(): Runtime was destroyed previously",
              downloader->error().errorString());
    EXPECT_EQ(Item(), downloader->item());
}

TEST_F(DownloadTest, download_error)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("download_error")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    // Signal must arrive.
    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));
    }

    EXPECT_FALSE(downloader->isValid());
    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::ResourceError, downloader->error().type());
    EXPECT_EQ("ResourceError: test error", downloader->error().errorString());
    EXPECT_EQ(42, downloader->error().errorCode());
    EXPECT_EQ(Item(), downloader->item());

    // For coverage: call finishDownload() while in the Error state.
    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        downloader->finishDownload();
        EXPECT_FALSE(spy.wait(1000));
    }
}

TEST_F(DownloadTest, finish_too_soon)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);

    downloader->finishDownload();

    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));

    EXPECT_FALSE(downloader->isValid());
    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::LogicError, downloader->error().type());
    EXPECT_EQ("LogicError: Downloader::finishDownload(): cannot finalize while Downloader is not in the Ready state",
              downloader->error().errorString());
}

TEST_F(DownloadTest, finish_runtime_destroyed)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Ready, qvariant_cast<Downloader::Status>(arg.at(0)));
    }

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime

    downloader->finishDownload();

    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));

    EXPECT_FALSE(downloader->isValid());
    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::RuntimeDestroyed, downloader->error().type());
    EXPECT_EQ("Downloader::finishDownload(): Runtime was destroyed previously", downloader->error().message());
}

TEST_F(DownloadTest, finish_runtime_destroyed_while_reply_outstanding)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("finish_download_slow")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Ready, qvariant_cast<Downloader::Status>(arg.at(0)));
    }

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);

    downloader->finishDownload();

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime, provider still sleeping

    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));

    EXPECT_FALSE(downloader->isValid());
    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::RuntimeDestroyed, downloader->error().type());
    EXPECT_EQ("Downloader::finishDownload(): Runtime was destroyed previously", downloader->error().message());
}

TEST_F(DownloadTest, finish_twice)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Ready, qvariant_cast<Downloader::Status>(arg.at(0)));
    }

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);

    downloader->finishDownload();
    downloader->finishDownload();

    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Downloader::Status::Finished, qvariant_cast<Downloader::Status>(arg.at(0)));

    EXPECT_EQ(Downloader::Status::Finished, downloader->status());
}

TEST_F(DownloadTest, finish_error)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("finish_download_error")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Ready, qvariant_cast<Downloader::Status>(arg.at(0)));
    }

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);

    downloader->finishDownload();

    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));

    EXPECT_FALSE(downloader->isValid());
    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::NotExists, downloader->error().type());
    EXPECT_EQ("no such item", downloader->error().message());
    EXPECT_EQ("item_id", downloader->error().itemId());
    EXPECT_EQ("item_id", downloader->error().itemName());
    EXPECT_EQ("NotExists", downloader->error().name());
}

TEST_F(DownloadTest, wrong_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<Downloader> downloader(root.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_FALSE(downloader->isValid());
    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::Type::LogicError, downloader->error().type());
    EXPECT_EQ("Item::createDownloader(): cannot download a folder", downloader->error().message());

    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));
    }
}

TEST_F(DownloadTest, conflict)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::ErrorIfConflict));
    EXPECT_TRUE(downloader->isValid());

    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));
    }

    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::Type::Conflict, downloader->error().type());
    EXPECT_EQ("download(): etag mismatch", downloader->error().message());
}

TEST_F(DownloadTest, cancel)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("finish_download_slow_error")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Ready, qvariant_cast<Downloader::Status>(arg.at(0)));
    }

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);

    downloader->finishDownload();

    downloader->cancel();
    downloader->cancel();  // Second time for coverage

    downloader->finishDownload();  // Second time for coverage

    EXPECT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Downloader::Status::Cancelled, qvariant_cast<Downloader::Status>(arg.at(0)));

    EXPECT_EQ(Downloader::Status::Cancelled, downloader->status());
    EXPECT_EQ(StorageError::Type::Cancelled, downloader->error().type());
    EXPECT_EQ("Downloader::cancel(): download was cancelled", downloader->error().message());

    // We wait here to get coverage for when the reply to a finishDownload() call
    // finds the downloader in a final state.
    EXPECT_FALSE(spy.wait(2000));
}

TEST_F(DownloadTest, cancel_runtime_destroyed)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("finish_download_slow_error")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Downloader> downloader(child.createDownloader(Item::ConflictPolicy::IgnoreConflict));
    EXPECT_TRUE(downloader->isValid());

    {
        QSignalSpy spy(downloader.get(), &Downloader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Downloader::Status::Ready, qvariant_cast<Downloader::Status>(arg.at(0)));
    }

    QSignalSpy spy(downloader.get(), &Downloader::statusChanged);

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime

    downloader->cancel();

    if (spy.count() == 0)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto arg = spy.takeFirst();
    EXPECT_EQ(Downloader::Status::Error, qvariant_cast<Downloader::Status>(arg.at(0)));

    EXPECT_EQ(Downloader::Status::Error, downloader->status());
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, downloader->error().type());
    EXPECT_EQ("Downloader::cancel(): Runtime was destroyed previously", downloader->error().message());
}

TEST_F(UploadTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    QByteArray contents("Hello world", -1);
    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, contents.size()));
    EXPECT_TRUE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Loading, uploader->status());
    EXPECT_EQ(StorageError::NoError, uploader->error().type());
    EXPECT_EQ(Item(), uploader->item());
    EXPECT_EQ(Item::ConflictPolicy::IgnoreConflict, uploader->policy());
    EXPECT_EQ(contents.size(), uploader->sizeInBytes());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    EXPECT_EQ(contents.size(), uploader->write(contents));

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    uploader->finishUpload();
    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Finished, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_EQ(Uploader::Status::Finished, uploader->status());
    EXPECT_EQ(child, uploader->item());
}

TEST_F(UploadTest, abandoned)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 5));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }
}

TEST_F(UploadTest, runtime_destroyed)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 20));
    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Downloader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::RuntimeDestroyed, uploader->error().type());
    EXPECT_EQ("RuntimeDestroyed: Item::createUploader(): Runtime was destroyed previously",
              uploader->error().errorString());
    EXPECT_EQ(Item(), uploader->item());

    // Signal must arrive.
    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));
    }
}

TEST_F(UploadTest, runtime_destroyed_while_upload_running)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("upload_slow")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::ErrorIfConflict, 20));
    EXPECT_TRUE(uploader->isValid());

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime, provider still sleeping

    // Signal must arrive.
    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::RuntimeDestroyed, uploader->error().type());
    EXPECT_EQ("RuntimeDestroyed: Item::createUploader(): Runtime was destroyed previously",
              uploader->error().errorString());
    EXPECT_EQ(Item(), uploader->item());
}

TEST_F(UploadTest, upload_error)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("upload_error")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::ErrorIfConflict, 100));
    EXPECT_TRUE(uploader->isValid());

    // Signal must arrive.
    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::Conflict, uploader->error().type());
    EXPECT_EQ("Conflict: version mismatch", uploader->error().errorString());
    EXPECT_EQ(Item(), uploader->item());

    // For coverage: call finishUpload() while in the Error state.
    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        uploader->finishUpload();
        EXPECT_FALSE(spy.wait(1000));
    }
}

TEST_F(UploadTest, finish_too_soon)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_TRUE(uploader->isValid());

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);

    uploader->finishUpload();

    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::LogicError, uploader->error().type());
    EXPECT_EQ("LogicError: Uploader::finishUpload(): cannot finalize while Uploader is not in the Ready state",
              uploader->error().errorString());
}

TEST_F(UploadTest, finish_runtime_destroyed)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime

    uploader->finishUpload();

    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::RuntimeDestroyed, uploader->error().type());
    EXPECT_EQ("Uploader::finishUpload(): Runtime was destroyed previously", uploader->error().message());
}

TEST_F(UploadTest, finish_runtime_destroyed_while_reply_outstanding)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("finish_upload_slow")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);

    uploader->finishUpload();

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime, provider still sleeping

    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::RuntimeDestroyed, uploader->error().type());
    EXPECT_EQ("Uploader::finishUpload(): Runtime was destroyed previously", uploader->error().message());
}

TEST_F(UploadTest, finish_twice)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);

    uploader->finishUpload();
    uploader->finishUpload();

    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Finished, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_EQ(Uploader::Status::Finished, uploader->status());
}

TEST_F(UploadTest, finish_error)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("finish_upload_error")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);

    uploader->finishUpload();

    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::ResourceError, uploader->error().type());
    EXPECT_EQ("out of memory", uploader->error().message());
    EXPECT_EQ(99, uploader->error().errorCode());
}

TEST_F(UploadTest, wrong_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    unique_ptr<Uploader> uploader(root.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::Type::LogicError, uploader->error().type());
    EXPECT_EQ("Item::createUploader(): cannot upload to a folder", uploader->error().message());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));
    }
}

TEST_F(UploadTest, wrong_size)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, -1));
    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::Type::InvalidArgument, uploader->error().type());
    EXPECT_EQ("Item::createUploader(): size must be >= 0", uploader->error().message());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));
    }
}

TEST_F(UploadTest, wrong_return_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("upload_returns_dir")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    uploader->finishUpload();
    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::Type::LocalCommsError, uploader->error().type());
    EXPECT_EQ("Item::createUploader(): impossible folder item returned by provider", uploader->error().message());
}

TEST_F(UploadTest, cancel_success)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    uploader->cancel();

    EXPECT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Cancelled, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_EQ(Uploader::Status::Cancelled, uploader->status());
    EXPECT_EQ(StorageError::Type::Cancelled, uploader->error().type());
    EXPECT_EQ("Uploader::cancel(): upload was cancelled", uploader->error().message());

    // We wait here to get coverage for when the successful reply for the cancel message.
    EXPECT_FALSE(spy.wait(2000));
}

TEST_F(UploadTest, cancel_error)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("finish_upload_slow_error")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);

    uploader->finishUpload();

    uploader->cancel();
    uploader->cancel();  // Second time for coverage

    uploader->finishUpload();  // Second time for coverage

    EXPECT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Cancelled, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_EQ(Uploader::Status::Cancelled, uploader->status());
    EXPECT_EQ(StorageError::Type::Cancelled, uploader->error().type());
    EXPECT_EQ("Uploader::cancel(): upload was cancelled", uploader->error().message());

    // We wait here to get coverage for when the reply to a finishUpload() call
    // finds the uploader in a final state.
    EXPECT_FALSE(spy.wait(2000));
}

TEST_F(UploadTest, cancel_runtime_destroyed)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("finish_upload_slow_error")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    unique_ptr<Uploader> uploader(child.createUploader(Item::ConflictPolicy::IgnoreConflict, 0));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime

    uploader->cancel();

    if (spy.count() == 0)
    {
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, uploader->error().type());
    EXPECT_EQ("Uploader::cancel(): Runtime was destroyed previously", uploader->error().message());
}

TEST_F(CreateFileTest, basic)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    QByteArray contents("Hello world", -1);
    unique_ptr<Uploader> uploader(root.createFile("Child",
                                                  Item::ConflictPolicy::IgnoreConflict,
                                                  contents.size(),
                                                  ""));
    EXPECT_TRUE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Loading, uploader->status());
    EXPECT_EQ(StorageError::NoError, uploader->error().type());
    EXPECT_EQ(Item(), uploader->item());
    EXPECT_EQ(Item::ConflictPolicy::IgnoreConflict, uploader->policy());
    EXPECT_EQ(contents.size(), uploader->sizeInBytes());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    EXPECT_EQ(contents.size(), uploader->write(contents));

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    uploader->finishUpload();
    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Finished, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_EQ(Uploader::Status::Finished, uploader->status());
    EXPECT_EQ(child, uploader->item());
}

TEST_F(CreateFileTest, runtime_destroyed)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    EXPECT_EQ(StorageError::Type::NoError, runtime_->shutdown().type());  // Destroy runtime

    QByteArray contents("Hello world", -1);
    unique_ptr<Uploader> uploader(root.createFile("Child",
                                                  Item::ConflictPolicy::IgnoreConflict,
                                                  contents.size(),
                                                  ""));
    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::Type::RuntimeDestroyed, uploader->error().type());
    EXPECT_EQ("Item::createFile(): Runtime was destroyed previously", uploader->error().message());

    // Signal must be received.
    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));
}

TEST_F(CreateFileTest, wrong_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    QByteArray contents("Hello world", -1);
    unique_ptr<Uploader> uploader(child.createFile("somefile",
                                                   Item::ConflictPolicy::IgnoreConflict,
                                                   contents.size(),
                                                   ""));
    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::Type::LogicError, uploader->error().type());
    EXPECT_EQ("Item::createFile(): cannot create a file with a file as the parent", uploader->error().message());

    // Signal must be received.
    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));
}

TEST_F(CreateFileTest, bad_name)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    QByteArray contents("Hello world", -1);
    unique_ptr<Uploader> uploader(root.createFile("",
                                                   Item::ConflictPolicy::IgnoreConflict,
                                                   contents.size(),
                                                   ""));
    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::Type::InvalidArgument, uploader->error().type());
    EXPECT_EQ("Item::createFile(): name cannot be empty", uploader->error().message());

    // Signal must be received.
    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));
}

TEST_F(CreateFileTest, bad_size)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider()));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    QByteArray contents("Hello world", -1);
    unique_ptr<Uploader> uploader(root.createFile("some_file",
                                                   Item::ConflictPolicy::IgnoreConflict,
                                                   -1,
                                                   ""));
    EXPECT_FALSE(uploader->isValid());
    EXPECT_EQ(Uploader::Status::Error, uploader->status());
    EXPECT_EQ(StorageError::Type::InvalidArgument, uploader->error().type());
    EXPECT_EQ("Item::createFile(): size must be >= 0", uploader->error().message());

    // Signal must be received.
    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    spy.wait(SIGNAL_WAIT_TIME);
    ASSERT_EQ(1, spy.count());
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));
}

TEST_F(CreateFileTest, bad_return_type)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("upload_returns_dir")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    Item child;
    {
        unique_ptr<ItemJob> j(acc_.get("child_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        child = j->item();
    }

    QByteArray contents("Hello world", -1);
    unique_ptr<Uploader> uploader(root.createFile("some_file",
                                                   Item::ConflictPolicy::IgnoreConflict,
                                                   contents.size(),
                                                   ""));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    EXPECT_EQ(contents.size(), uploader->write(contents));

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    uploader->finishUpload();
    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_EQ(StorageError::Type::LocalCommsError, uploader->error().type());
    EXPECT_EQ("Item::createFile(): impossible folder item returned by provider", uploader->error().message());
}

TEST_F(CreateFileTest, exists)
{
    set_provider(unique_ptr<provider::ProviderBase>(new MockProvider("create_file_exists")));

    Item root;
    {
        unique_ptr<ItemJob> j(acc_.get("root_id"));
        QSignalSpy spy(j.get(), &ItemJob::statusChanged);
        spy.wait(SIGNAL_WAIT_TIME);
        root = j->item();
    }

    QByteArray contents("Hello world", -1);
    unique_ptr<Uploader> uploader(root.createFile("Child",
                                                   Item::ConflictPolicy::ErrorIfConflict,
                                                   contents.size(),
                                                   ""));
    EXPECT_TRUE(uploader->isValid());

    {
        QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        auto arg = spy.takeFirst();
        EXPECT_EQ(Uploader::Status::Ready, qvariant_cast<Uploader::Status>(arg.at(0)));
    }

    EXPECT_EQ(contents.size(), uploader->write(contents));

    QSignalSpy spy(uploader.get(), &Uploader::statusChanged);
    uploader->finishUpload();
    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    auto arg = spy.takeFirst();
    EXPECT_EQ(Uploader::Status::Error, qvariant_cast<Uploader::Status>(arg.at(0)));

    EXPECT_EQ(StorageError::Type::Exists, uploader->error().type());
    EXPECT_EQ("file exists", uploader->error().message());
    EXPECT_EQ("child_id", uploader->error().itemId());
    EXPECT_EQ("Child", uploader->error().itemName());
    EXPECT_EQ(Item::ConflictPolicy::ErrorIfConflict, uploader->policy());
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
