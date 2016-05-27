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

using namespace unity::storage::qt::client;
using namespace std;

static constexpr int SIGNAL_WAIT_TIME = 1000;

TEST(runtime, lifecycle)
{
    auto runtime = Runtime::create();
    runtime->shutdown();
    runtime->shutdown();  // Just to show that this is safe.
}

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
    EXPECT_EQ(1, accounts.size());
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
    EXPECT_EQ(1, roots.size());
    return roots[0];
}

TEST(runtime, basic)
{
    auto runtime = Runtime::create();

    auto accounts_fut = runtime->accounts();
    {
        QFutureWatcher<QVector<Account::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(accounts_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto accounts = accounts_fut.result();
    EXPECT_EQ(1, accounts.size());
    auto acc = accounts[0];
    EXPECT_EQ(runtime.get(), acc->runtime());
    auto owner = acc->owner();
    EXPECT_EQ(QString(g_get_user_name()), owner);
    auto owner_id = acc->owner_id();
    EXPECT_EQ(QString::number(getuid()), owner_id);
    auto description = acc->description();
    EXPECT_EQ(description, QString("Account for ") + owner + " (" + owner_id + ")");
}

TEST(runtime, accounts)
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
    EXPECT_EQ(1, roots.size());
}

TEST(root, basic)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    EXPECT_EQ(acc.get(), root->account());
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
    EXPECT_EQ(parent->name(), root->name());
    EXPECT_EQ(parent->native_identity(), root->native_identity());

    // get(<root-path> must return the root.
    auto get_fut = root->get(root->native_identity());
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(get_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto item = get_fut.result();
    EXPECT_EQ("", item->name());
    EXPECT_NE(nullptr, dynamic_pointer_cast<Root>(item));

    // Free and used space can be anything, but must be non-zero.
    auto free_space_fut = root->free_space_bytes();
    {
        QFutureWatcher<int64_t> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(free_space_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto free_space = free_space_fut.result();
    cerr << "bytes free: " << free_space << endl;
    EXPECT_NE(0, free_space);

    auto used_space_fut = root->used_space_bytes();
    {
        QFutureWatcher<int64_t> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(used_space_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto used_space = used_space_fut.result();
    cerr << "bytes used: " << used_space << endl;
    EXPECT_NE(0, used_space);
}

TEST(folder, basic)
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

    // Create a file and check that it was created with size 0.
    auto create_file_fut = root->create_file("file1");
    {
        QFutureWatcher<Uploader::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(create_file_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto file = create_file_fut.result()->file();
    EXPECT_EQ("file1", file->name());
    EXPECT_EQ(0, file->size());
    EXPECT_EQ(root->native_identity() + "/file1", file->native_identity());
}

int main(int argc, char** argv)
{
    boost::filesystem::remove_all(TEST_DIR "/storage-framework");
    setenv("STORAGE_FRAMEWORK_ROOT", TEST_DIR, true);

    QCoreApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
