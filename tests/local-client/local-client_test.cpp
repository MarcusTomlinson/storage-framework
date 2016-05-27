#include <unity/storage/qt/client/client-api.h>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QFutureWatcher>
#include <QSignalSpy>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <glib.h>
#pragma GCC diagnostic pop

#include <iostream> // TODO: remove this

using namespace unity::storage;
using namespace unity::storage::qt::client;
using namespace std;

static constexpr int SIGNAL_WAIT_TIME = 1000;

Account::SPtr get_account(Runtime::SPtr const& runtime)
{
    auto accounts_fut = runtime->accounts();
    {
        QFutureWatcher<QVector<Account::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(accounts_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto accounts = accounts_fut.result();
    assert(accounts.size() == 1);
    return accounts[0];
}

Root::SPtr get_root(Runtime::SPtr const& runtime)
{
    auto acc = get_account(runtime);
    auto roots_fut = acc->roots();
    {
        QFutureWatcher<QVector<Root::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(roots_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto roots = roots_fut.result();
    assert(roots.size() == 1);
    return roots[0];
}

Uploader::SPtr create_file(QString const& name, Folder::SPtr const& parent)
{
    auto create_file_fut = parent->create_file(name);
    QFutureWatcher<Uploader::SPtr> w;
    QSignalSpy spy(&w, &decltype(w)::finished);
    w.setFuture(create_file_fut);
    spy.wait(SIGNAL_WAIT_TIME);
    return create_file_fut.result();
}

Folder::SPtr create_folder(QString const& name, Folder::SPtr const& parent)
{
    auto create_folder_fut = parent->create_folder(name);
    QFutureWatcher<Folder::SPtr> w;
    QSignalSpy spy(&w, &decltype(w)::finished);
    w.setFuture(create_folder_fut);
    spy.wait(SIGNAL_WAIT_TIME);
    return create_folder_fut.result();
}

TEST(runtime, lifecycle)
{
    auto runtime = Runtime::create();
    runtime->shutdown();
    runtime->shutdown();  // Just to show that this is safe.
}

TEST(Runtime, basic)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    EXPECT_EQ(runtime.get(), acc->runtime());
    auto owner = acc->owner();
    EXPECT_EQ(QString(g_get_user_name()), owner);
    auto owner_id = acc->owner_id();
    EXPECT_EQ(QString::number(getuid()), owner_id);
    auto description = acc->description();
    EXPECT_EQ(description, QString("Account for ") + owner + " (" + owner_id + ")");
}

TEST(Runtime, accounts)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto roots_fut = acc->roots();
    {
        QFutureWatcher<QVector<Root::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(roots_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto roots = roots_fut.result();
    EXPECT_EQ(1, roots.size());

    // Get roots again, to get coverage for lazy initialization.
    roots_fut = acc->roots();
    {
        QFutureWatcher<QVector<Root::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(roots_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    roots = roots_fut.result();
    ASSERT_EQ(1, roots.size());
}

TEST(Root, basic)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    EXPECT_EQ(acc.get(), root->account());
    EXPECT_EQ(ItemType::root, root->type());
    EXPECT_EQ("", root->name());

    auto parents_fut = root->parents();
    {
        QFutureWatcher<QVector<Folder::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(parents_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto parents = parents_fut.result();
    ASSERT_EQ(1, parents.size());
    auto parent = parents[0];
    EXPECT_EQ(parent->type(), root->type());
    EXPECT_EQ(parent->name(), root->name());
    EXPECT_EQ(parent->native_identity(), root->native_identity());

    // get(<root-path>) must return the root.
    auto get_fut = root->get(root->native_identity());
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(get_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto item = get_fut.result();
    EXPECT_NE(nullptr, dynamic_pointer_cast<Root>(item));
    EXPECT_TRUE(root->equal_to(item));

    // Free and used space can be anything, but must be > 0.
    auto free_space_fut = root->free_space_bytes();
    {
        QFutureWatcher<int64_t> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(free_space_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto free_space = free_space_fut.result();
    cerr << "bytes free: " << free_space << endl;
    EXPECT_GT(free_space, 0);

    auto used_space_fut = root->used_space_bytes();
    {
        QFutureWatcher<int64_t> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(used_space_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto used_space = used_space_fut.result();
    cerr << "bytes used: " << used_space << endl;
    EXPECT_GT(used_space, 0);
}

TEST(Folder, basic)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);

    // Test always starts out with an empty root.
    auto list_fut = root->list();
    {
        QFutureWatcher<QVector<Item::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(list_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto items = list_fut.result();
    EXPECT_TRUE(items.isEmpty());

    // Create a file and check that it was created with correct type, name, and size 0.
    auto file = create_file("file1", root)->file();
    EXPECT_EQ(ItemType::file, file->type());
    EXPECT_EQ("file1", file->name());
    EXPECT_EQ(0, file->size());
    EXPECT_EQ(root->native_identity() + "/file1", file->native_identity());

    // Create a folder and check that it was created with correct type and name.
    auto folder = create_folder("folder1", root);
    EXPECT_EQ(ItemType::folder, folder->type());
    EXPECT_EQ("folder1", folder->name());
    EXPECT_EQ(root->native_identity() + "/folder1", folder->native_identity());

    // Check that we can find both file1 and folder1.
    auto lookup_fut = root->lookup("file1");
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(lookup_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto item = lookup_fut.result();
    file = dynamic_pointer_cast<File>(item);
    ASSERT_NE(nullptr, file);
    EXPECT_EQ("file1", file->name());
    EXPECT_EQ(0, file->size());

    lookup_fut = root->lookup("folder1");
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(lookup_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    item = lookup_fut.result();
    folder = dynamic_pointer_cast<Folder>(item);
    ASSERT_NE(nullptr, folder);
    ASSERT_EQ(nullptr, dynamic_pointer_cast<Root>(folder));
    EXPECT_EQ("folder1", folder->name());

    // Check that list() returns file1 and folder1.
    list_fut = root->list();
    {
        QFutureWatcher<QVector<Item::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(list_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    items = list_fut.result();
    ASSERT_EQ(2, items.size());
    auto left = items[0];
    auto right = items[1];
    ASSERT_TRUE((dynamic_pointer_cast<File>(left) && dynamic_pointer_cast<Folder>(right))
                ||
                (dynamic_pointer_cast<File>(right) && dynamic_pointer_cast<Folder>(left)));
    if (dynamic_pointer_cast<File>(left))
    {
        file = dynamic_pointer_cast<File>(left);
        folder = dynamic_pointer_cast<Folder>(right);
    }
    else
    {
        file = dynamic_pointer_cast<File>(right);
        folder = dynamic_pointer_cast<Folder>(left);
    }
    EXPECT_EQ("file1", file->name());
    EXPECT_EQ("folder1", folder->name());
    EXPECT_TRUE(file->root()->equal_to(root));
    EXPECT_TRUE(folder->root()->equal_to(root));

    // Destroy the file and check that only the directory is left.
    auto destroy_fut = file->destroy();
    {
        QFutureWatcher<void> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(destroy_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    list_fut = root->list();
    {
        QFutureWatcher<QVector<Item::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(list_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    items = list_fut.result();
    ASSERT_EQ(1, items.size());
    folder = dynamic_pointer_cast<Folder>(items[0]);
    ASSERT_NE(nullptr, folder);
    EXPECT_EQ("folder1", folder->name());;

    // Destroy the folder and check that the root is empty.
    destroy_fut = folder->destroy();
    {
        QFutureWatcher<void> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(destroy_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    list_fut = root->list();
    {
        QFutureWatcher<QVector<Item::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(list_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    items = list_fut.result();
    ASSERT_EQ(0, items.size());
}

TEST(Item, comparison)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);

    // Create two files.
    auto file1 = create_file("file1", root)->file();
    auto file2 = create_file("file2", root)->file();

    EXPECT_FALSE(file1->equal_to(file2));

    // Retrieve file1 via lookup, so we get a different proxy.
    auto lookup_fut = root->lookup("file1");
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(lookup_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto item = lookup_fut.result();
    auto other_file1 = dynamic_pointer_cast<File>(item);
    EXPECT_NE(file1, other_file1);             // Compares shared_ptr values
    EXPECT_TRUE(file1->equal_to(other_file1));  // Deep comparison
}

int main(int argc, char** argv)
{
    boost::filesystem::remove_all(TEST_DIR "/storage-framework");
    setenv("STORAGE_FRAMEWORK_ROOT", TEST_DIR, true);

    QCoreApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
