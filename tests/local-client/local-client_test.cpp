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
    auto accounts = runtime->accounts().result();
    assert(accounts.size() == 1);
    return accounts[0];
}

Root::SPtr get_root(Runtime::SPtr const& runtime)
{
    auto acc = get_account(runtime);
    auto roots = acc->roots().result();
    assert(roots.size() == 1);
    return roots[0];
}

Folder::SPtr get_parent(Item::SPtr const& item)
{
    auto parents = item->parents().result();
    assert(parents.size() == 1);
    return parents[0];
}

void clear_folder(Folder::SPtr folder)
{
    auto items = folder->list().result();
    for (auto i : items)
    {
        i->destroy().waitForFinished();
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
    auto roots = acc->roots().result();
    EXPECT_EQ(1, roots.size());

    // Get roots again, to get coverage for lazy initialization.
    roots = acc->roots().result();
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
    auto item = root->get(root->native_identity()).result();
    EXPECT_NE(nullptr, dynamic_pointer_cast<Root>(item));
    EXPECT_TRUE(root->equal_to(item));

    // Free and used space can be anything, but must be > 0.
    auto free_space = root->free_space_bytes().result();
    cerr << "bytes free: " << free_space << endl;
    EXPECT_GT(free_space, 0);

    auto used_space = root->used_space_bytes().result();
    cerr << "bytes used: " << used_space << endl;
    EXPECT_GT(used_space, 0);
}

TEST(Folder, basic)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    auto items = root->list().result();
    EXPECT_TRUE(items.isEmpty());

    // Create a file and check that it was created with correct type, name, and size 0.
    auto file = root->create_file("file1").result()->file();
    EXPECT_EQ(ItemType::file, file->type());
    EXPECT_EQ("file1", file->name());
    EXPECT_EQ(0, file->size());
    EXPECT_EQ(root->native_identity() + "/file1", file->native_identity());

    // Create a folder and check that it was created with correct type and name.
    auto folder = root->create_folder("folder1").result();
    EXPECT_EQ(ItemType::folder, folder->type());
    EXPECT_EQ("folder1", folder->name());
    EXPECT_EQ(root->native_identity() + "/folder1", folder->native_identity());

    // Check that we can find both file1 and folder1.
    auto item = root->lookup("file1").result();
    file = dynamic_pointer_cast<File>(item);
    ASSERT_NE(nullptr, file);
    EXPECT_EQ("file1", file->name());
    EXPECT_EQ(0, file->size());

    item = root->lookup("folder1").result();
    folder = dynamic_pointer_cast<Folder>(item);
    ASSERT_NE(nullptr, folder);
    ASSERT_EQ(nullptr, dynamic_pointer_cast<Root>(folder));
    EXPECT_EQ("folder1", folder->name());

    // Check that list() returns file1 and folder1.
    items = root->list();
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
    file->destroy().waitForFinished();
    items = root->list().result();
    ASSERT_EQ(1, items.size());
    folder = dynamic_pointer_cast<Folder>(items[0]);
    ASSERT_NE(nullptr, folder);
    EXPECT_EQ("folder1", folder->name());;

    // Destroy the folder and check that the root is empty.
    folder->destroy().waitForFinished();
    items = root->list().result();
    ASSERT_EQ(0, items.size());
}

TEST(Folder, nested)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    auto d1 = root->create_folder("d1").result();
    auto d2 = d1->create_folder("d2").result();

    // Parent of d2 must be d1.
    EXPECT_TRUE(get_parent(d2)->equal_to(d1));

    // Destroy is recursive
    d1->destroy().waitForFinished();
    auto items = root->list().result();
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
        auto uploader = root->create_file("new_file").result();
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

        auto item = root->lookup("file").result();
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
        EXPECT_TRUE(file->equal_to(downloader->file()));

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

        auto item = root->lookup("file").result();
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
        EXPECT_TRUE(file->equal_to(downloader->file()));

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

        auto item = root->lookup("file").result();
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
        EXPECT_TRUE(file->equal_to(downloader->file()));

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

        auto item = root->lookup("file").result();
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
        EXPECT_TRUE(file->equal_to(downloader->file()));

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
    auto f1 = root->create_file("f1").result()->file();
    auto f2 = f1->move(root, "f2").result();
    EXPECT_EQ("f2", f2->name());
    EXPECT_THROW(f1->name(), DestroyedException);

    // File must be found under new name.
    auto items = root->list().result();
    ASSERT_EQ(1, items.size());
    f2 = dynamic_pointer_cast<File>(items[0]);
    ASSERT_FALSE(f2 == nullptr);

    // Make a folder and move f2 into it.
    auto folder = root->create_folder("folder").result();
    f2 = f2->move(folder, "f2").result();
    EXPECT_TRUE(get_parent(f2)->equal_to(folder));

    // Move the folder
    auto item = folder->move(root, "folder2").result();
    folder = dynamic_pointer_cast<Folder>(item);
    EXPECT_EQ("folder2", folder->name());
}

TEST(Item, copy)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    string const contents = "hello\n";
    write_file(root, "file", contents);

    auto item = root->lookup("file").result();
    auto copied_item = item->copy(root, "copy_of_file").result();
    EXPECT_EQ("copy_of_file", copied_item->name());
    File::SPtr copied_file = dynamic_pointer_cast<File>(item);
    ASSERT_NE(nullptr, copied_file);
    EXPECT_EQ(contents.size(), copied_file->size());
}

TEST(Item, recursive_copy)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    // Create the following structure:
    // folder
    // folder/empty_folder
    // folder/non_empty_folder
    // folder/non_empty_folder/nested_file
    // folder/file

    string root_path = root->native_identity().toStdString();
    ASSERT_EQ(0, mkdir((root_path + "/folder").c_str(), 0700));
    ASSERT_EQ(0, mkdir((root_path + "/folder/empty_folder").c_str(), 0700));
    ASSERT_EQ(0, mkdir((root_path + "/folder/non_empty_folder").c_str(), 0700));
    ofstream(root_path + "/folder/non_empty_folder/nested_file");
    ofstream(root_path + "/folder/file");

    // Copy folder to folder2
    auto folder = dynamic_pointer_cast<Folder>(root->lookup("folder").result());
    ASSERT_NE(nullptr, folder);
    auto item = folder->copy(root, "folder2").result();

    // Verify that folder2 now contains the same structure as folder.
    auto folder2 = dynamic_pointer_cast<Folder>(item);
    ASSERT_NE(nullptr, folder2);
    EXPECT_NO_THROW(folder2->lookup("empty_folder").result());
    item = folder2->lookup("non_empty_folder").result();
    auto non_empty_folder = dynamic_pointer_cast<Folder>(item);
    ASSERT_NE(nullptr, non_empty_folder);
    EXPECT_NO_THROW(non_empty_folder->lookup("nested_file").result());
    EXPECT_NO_THROW(folder2->lookup("file").result());
}

TEST(Item, comparison)
{
    auto runtime = Runtime::create();

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    // Create two files.
    auto file1 = root->create_file("file1").result()->file();
    auto file2 = root->create_file("file2").result()->file();

    EXPECT_FALSE(file1->equal_to(file2));

    // Retrieve file1 via lookup, so we get a different proxy.
    auto item = root->lookup("file1").result();
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
