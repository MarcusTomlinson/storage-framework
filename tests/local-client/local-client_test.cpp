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

#include <fstream>

using namespace unity::storage;
using namespace unity::storage::qt::client;
using namespace std;

static constexpr int SIGNAL_WAIT_TIME = 1000;

// Bunch of helper function to reduce the amount of noise in the tests.

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

QVector<Item::SPtr> list_contents(Folder::SPtr const& folder)
{
    auto list_fut = folder->list();
    QFutureWatcher<QVector<Item::SPtr>> w;
    QSignalSpy spy(&w, &decltype(w)::finished);
    w.setFuture(list_fut);
    spy.wait(SIGNAL_WAIT_TIME);
    return list_fut.result();
}

Item::SPtr lookup(Folder::SPtr folder, QString const& name)
{
    auto lookup_fut = folder->lookup(name);
    QFutureWatcher<Item::SPtr> w;
    QSignalSpy spy(&w, &decltype(w)::finished);
    w.setFuture(lookup_fut);
    spy.wait(SIGNAL_WAIT_TIME);
    return lookup_fut.result();
}

Folder::SPtr get_parent(Item::SPtr const& item)
{
    auto parents_fut = item->parents();
    QFutureWatcher<QVector<Folder::SPtr>> w;
    QSignalSpy spy(&w, &decltype(w)::finished);
    w.setFuture(parents_fut);
    spy.wait(SIGNAL_WAIT_TIME);
    auto parents = parents_fut.result();
    assert(parents.size() == 1);
    return parents[0];
}

void destroy(Item::SPtr item)
{
    auto destroy_fut = item->destroy();
    QFutureWatcher<void> w;
    QSignalSpy spy(&w, &decltype(w)::finished);
    w.setFuture(destroy_fut);
    spy.wait(SIGNAL_WAIT_TIME);
    w.future().waitForFinished();
}

void clear_folder(Folder::SPtr folder)
{
    auto items = list_contents(folder);
    for (auto i : items)
    {
        destroy(i);
    }
}

bool content_matches(File::SPtr const& file, string const& expected)
{
    ifstream is(file->native_identity().toStdString(), ifstream::binary);
    is.seekg(0, is.end);
    string::size_type len = is.tellg();
    if (len != expected.size())
    {
        return false;
    }
    is.seekg(0, is.beg);
    string buf;
    buf.resize(expected.size());
    is.read(&buf[0], expected.size());
    if (!is.good())
    {
        return false;
    }
    return buf == expected;
}

void write_file(Folder::SPtr const& folder, string const& name, string const& contents)
{
    string ofile = folder->native_identity().toStdString() + "/" + name;
    ofstream os(ofile, ios::trunc | ios::binary);
    os << contents;
    assert(os.good());
    os.close();
}

TEST(Runtime, lifecycle)
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

    auto parent = get_parent(root);
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
    clear_folder(root);

    auto items = list_contents(root);
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
    auto item = lookup(root, "file1");
    file = dynamic_pointer_cast<File>(item);
    ASSERT_NE(nullptr, file);
    EXPECT_EQ("file1", file->name());
    EXPECT_EQ(0, file->size());

    item = lookup(root, "folder1");
    folder = dynamic_pointer_cast<Folder>(item);
    ASSERT_NE(nullptr, folder);
    ASSERT_EQ(nullptr, dynamic_pointer_cast<Root>(folder));
    EXPECT_EQ("folder1", folder->name());

    // Check that list() returns file1 and folder1.
    items = list_contents(root);
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

    // Parent of both file and folder must be the root.
    EXPECT_EQ("", get_parent(file)->name());
    EXPECT_EQ("", get_parent(folder)->name());

    // Destroy the file and check that only the directory is left.
    destroy(file);
    items = list_contents(root);
    ASSERT_EQ(1, items.size());
    folder = dynamic_pointer_cast<Folder>(items[0]);
    ASSERT_NE(nullptr, folder);
    EXPECT_EQ("folder1", folder->name());;

    // Destroy the folder and check that the root is empty.
    destroy(folder);
    items = list_contents(root);
    ASSERT_EQ(0, items.size());
}

TEST(Folder, nested)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    auto d1 = create_folder("d1", root);
    auto d2 = create_folder("d2", d1);

    // Parent of d2 must be d1.
    EXPECT_TRUE(get_parent(d2)->equal_to(d1));

    // Destroy is recursive
    destroy(d1);
    auto items = list_contents(root);
    ASSERT_EQ(0, items.size());
}

TEST(File, upload)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    File::SPtr file;
    {
        // Upload a few bytes.
        auto uploader = create_file("new_file", root);
        file = uploader->file();
        string const contents = "Hello\n";
        uploader->socket()->writeData(contents.c_str(), contents.size());
        auto finish_fut = uploader->finish_upload();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto state = finish_fut.result();
        EXPECT_EQ(TransferState::ok, state);
        EXPECT_EQ(contents.size(), uploader->file()->size());
        EXPECT_TRUE(content_matches(uploader->file(), contents));
    }

    {
        // Upload exactly CHUNK_SIZE bytes.
        auto uploader_fut = file->create_uploader(ConflictPolicy::overwrite);
        {
            QFutureWatcher<Uploader::SPtr> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(uploader_fut);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto uploader = uploader_fut.result();
        string contents(StorageSocket::CHUNK_SIZE, 'a');
        auto written = uploader->socket()->writeData(&contents[0], contents.size());
        EXPECT_EQ(written, contents.size());

        auto finish_fut = uploader->finish_upload();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto state = finish_fut.result();
        EXPECT_EQ(TransferState::ok, state);
        EXPECT_EQ(contents.size(), uploader->file()->size());
        EXPECT_TRUE(content_matches(uploader->file(), contents));
    }

    {
        // Upload CHUNK_SIZE + 1 bytes.
        auto uploader_fut = file->create_uploader(ConflictPolicy::overwrite);
        {
            QFutureWatcher<Uploader::SPtr> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(uploader_fut);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto uploader = uploader_fut.result();
        string contents(StorageSocket::CHUNK_SIZE + 1, 'a');
        auto written = uploader->socket()->writeData(&contents[0], contents.size());
        EXPECT_EQ(written, contents.size());

        auto finish_fut = uploader->finish_upload();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        auto state = finish_fut.result();
        EXPECT_EQ(TransferState::ok, state);
        EXPECT_EQ(contents.size(), uploader->file()->size());
        EXPECT_TRUE(content_matches(uploader->file(), contents));
    }

    {
        // Don't upload anything.
        auto uploader_fut = file->create_uploader(ConflictPolicy::overwrite);
        {
            QFutureWatcher<Uploader::SPtr> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(uploader_fut);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto uploader = uploader_fut.result();
        // No write here.
        auto finish_fut = uploader->finish_upload();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        auto state = finish_fut.result();
        EXPECT_EQ(TransferState::ok, state);
        EXPECT_EQ(0, uploader->file()->size());
    }
}

TEST(File, download)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    {
        // Download a few bytes.
        string const contents = "hello\n";
        write_file(root, "file", contents);

        auto item = lookup(root, "file");
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader_fut = file->create_downloader();
        {
            QFutureWatcher<Downloader::SPtr> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(downloader_fut);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto downloader = downloader_fut.result();
        {
            QSignalSpy spy(downloader->socket().get(), &QIODevice::readyRead);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        string buf;
        buf.resize(StorageSocket::CHUNK_SIZE);
        EXPECT_EQ(contents.size(), downloader->socket()->readData(&buf[0], buf.size()));
        buf.resize(contents.size());

        auto finish_fut = downloader->finish_download();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        auto state = finish_fut.result();
        EXPECT_EQ(TransferState::ok, state);
        EXPECT_EQ(contents, buf);
    }

    {
        // Download exactly CHUNK_SIZE bytes.
        string const contents(StorageSocket::CHUNK_SIZE, 'a');
        write_file(root, "file", contents);

        auto item = lookup(root, "file");
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader_fut = file->create_downloader();
        {
            QFutureWatcher<Downloader::SPtr> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(downloader_fut);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto downloader = downloader_fut.result();
        {
            QSignalSpy spy(downloader->socket().get(), &QIODevice::readyRead);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        string buf;
        buf.resize(StorageSocket::CHUNK_SIZE);
        EXPECT_EQ(contents.size(), downloader->socket()->readData(&buf[0], buf.size()));
        buf.resize(contents.size());

        auto finish_fut = downloader->finish_download();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        auto state = finish_fut.result();
        EXPECT_EQ(TransferState::ok, state);
        EXPECT_EQ(contents, buf);
    }

    {
        // Download CHUNK_SIZE + 1 bytes.
        string const contents(StorageSocket::CHUNK_SIZE + 1, 'a');
        write_file(root, "file", contents);

        auto item = lookup(root, "file");
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader_fut = file->create_downloader();
        {
            QFutureWatcher<Downloader::SPtr> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(downloader_fut);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto downloader = downloader_fut.result();
        {
            QSignalSpy spy(downloader->socket().get(), &QLocalSocket::disconnected);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        string buf;
        buf.resize(StorageSocket::CHUNK_SIZE * 2);
        EXPECT_EQ(contents.size(), downloader->socket()->readData(&buf[0], buf.size()));
        buf.resize(contents.size());

        auto finish_fut = downloader->finish_download();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        auto state = finish_fut.result();
        EXPECT_EQ(TransferState::ok, state);
        EXPECT_EQ(contents, buf);
    }

    {
        // Download zero bytes.
        string const contents;
        write_file(root, "file", contents);

        auto item = lookup(root, "file");
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader_fut = file->create_downloader();
        {
            QFutureWatcher<Downloader::SPtr> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(downloader_fut);
            spy.wait(SIGNAL_WAIT_TIME);
        }
        auto downloader = downloader_fut.result();
        {
            QSignalSpy spy(downloader->socket().get(), &QLocalSocket::disconnected);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        // No read here.
        auto finish_fut = downloader->finish_download();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        auto state = finish_fut.result();
        EXPECT_EQ(TransferState::ok, state);
    }
}

TEST(Item, move)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    // Check that rename works within the same folder.
    auto f1 = create_file("f1", root)->file();
    auto move_fut = f1->move(root, "f2");
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(move_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    auto f2 = move_fut.result();
    EXPECT_EQ("f2", f2->name());
    EXPECT_THROW(f1->name(), DestroyedException);

    // File must be found under new name.
    auto items = list_contents(root);
    ASSERT_EQ(1, items.size());
    f2 = dynamic_pointer_cast<File>(items[0]);
    ASSERT_FALSE(f2 == nullptr);

    // Make a folder and move f2 into it.
    auto folder = create_folder("folder", root);
    move_fut = f2->move(folder, "f2");
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(move_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    f2 = move_fut.result();
    EXPECT_TRUE(get_parent(f2)->equal_to(folder));

    // Move the folder
    move_fut = folder->move(root, "folder2");
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(move_fut);
        spy.wait(SIGNAL_WAIT_TIME);
    }
    folder = dynamic_pointer_cast<Folder>(move_fut.result());
    EXPECT_EQ("folder2", folder->name());
}

TEST(Item, copy)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);
}

TEST(Item, comparison)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    // Create two files.
    auto file1 = create_file("file1", root)->file();
    auto file2 = create_file("file2", root)->file();

    EXPECT_FALSE(file1->equal_to(file2));

    // Retrieve file1 via lookup, so we get a different proxy.
    auto item = lookup(root, "file1");
    auto other_file1 = dynamic_pointer_cast<File>(item);
    EXPECT_NE(file1, other_file1);              // Compares shared_ptr values
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
