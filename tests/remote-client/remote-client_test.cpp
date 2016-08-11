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

#include <unity/storage/qt/client/client-api.h>

#include <utils/DBusEnvironment.h>

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QFile>
#include <QFutureWatcher>
#include <QSignalSpy>
#include <QTimer>

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

class RemoteClientTest : public ::testing::Test
{
public:
    QDBusConnection const& connection()
    {
        return dbus_->connection();
    }

protected:
    void SetUp() override
    {
        dbus_.reset(new DBusEnvironment);
        dbus_->add_demo_provider("com.canonical.scopes.mcloud_mcloud_mcloud");
        dbus_->start_services();
    }

    void TearDown() override
    {
        dbus_.reset();
    }

private:
    unique_ptr<DBusEnvironment> dbus_;
};

class RuntimeTest : public RemoteClientTest {};
class RootTest : public RemoteClientTest {};
class FolderTest : public RemoteClientTest {};
class FileTest : public RemoteClientTest {};
class ItemTest : public RemoteClientTest {};

Account::SPtr get_account(Runtime::SPtr const& runtime)
{
    auto accounts_fut = runtime->accounts();
    QFutureWatcher<QVector<Account::SPtr>> w;
    QSignalSpy spy(&w, &decltype(w)::finished);
    w.setFuture(accounts_fut);
    assert(spy.wait(SIGNAL_WAIT_TIME));

    auto accounts = accounts_fut.result();
    if (accounts.size() == 0)
    {
        qCritical() << "Cannot find any online account";
        qCritical() << "Configure at least one online account for a provider in System Settings -> Online Accounts";
        return nullptr;
    }
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
        assert(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto roots = roots_fut.result();
    assert(roots.size() >= 1);
    return roots[0];
}

Folder::SPtr get_parent(Item::SPtr const& item)
{
    assert(item->type() != ItemType::root);
    auto parents_fut = item->parents();
    {
        QFutureWatcher<QVector<Folder::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(parents_fut);
        assert(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto parents = parents_fut.result();
    assert(parents.size() >= 1);
    return parents[0];
}

void clear_folder(Folder::SPtr const& folder)
{
    auto list_fut = folder->list();
    {
        QFutureWatcher<QVector<Item::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(list_fut);
        assert(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto items = list_fut.result();
    assert(items.size() != 0);  // TODO: temporary hack for use with demo provider
    for (auto i : items)
    {
        auto delete_fut = i->delete_item();
        QFutureWatcher<void> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(delete_fut);
        assert(spy.wait(SIGNAL_WAIT_TIME));
    }
}

bool content_matches(File::SPtr const& file, QByteArray const& expected)
{
    QFile f(file->native_identity());
    assert(f.open(QIODevice::ReadOnly));
    QByteArray buf = f.readAll();
    return buf == expected;
}

void write_file(Folder::SPtr const& folder, QString const& name, QByteArray const& contents)
{
    QString ofile = folder->native_identity() + "/" + name;
    QFile f(ofile);
    assert(f.open(QIODevice::Truncate | QIODevice::WriteOnly));
    if (!contents.isEmpty())
    {
        assert(f.write(contents));
    }
}

TEST_F(RuntimeTest, lifecycle)
{
    auto runtime = Runtime::create(connection());
    runtime->shutdown();
    runtime->shutdown();  // Just to show that this is safe.
}

TEST_F(RuntimeTest, basic)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    EXPECT_EQ(runtime, acc->runtime());
    qDebug() << "owner:      " << acc->owner();
    qDebug() << "owner ID:   " << acc->owner_id();
    qDebug() << "description:" << acc->description();
}

TEST_F(RuntimeTest, roots)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    ASSERT_NE(nullptr, acc);
    auto roots_fut = acc->roots();
    {
        QFutureWatcher<QVector<Root::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(roots_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto roots = roots_fut.result();
    ASSERT_GE(roots.size(), 0);
    EXPECT_EQ("root_id", roots[0]->native_identity());
}

TEST_F(RootTest, basic)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    EXPECT_EQ("root_id", root->native_identity());
    EXPECT_EQ(acc, root->account());
    EXPECT_EQ(ItemType::root, root->type());
    EXPECT_EQ("Root", root->name());
    EXPECT_NE("", root->etag());

    auto parents = root->parents().result();
    EXPECT_TRUE(parents.isEmpty());
    EXPECT_TRUE(root->parent_ids().isEmpty());

    // get(<root-ID>) must return the root.
    auto get_fut = root->get(root->native_identity());
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(get_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
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
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto free_space = free_space_fut.result();
    cerr << "bytes free: " << free_space << endl;
    EXPECT_GT(free_space, 0);

    auto used_space_fut = root->used_space_bytes();
    {
        QFutureWatcher<int64_t> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(free_space_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto used_space = used_space_fut.result();
    cerr << "bytes used: " << used_space << endl;
    EXPECT_GT(used_space, 0);
}

TEST_F(FolderTest, basic)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    auto list_fut = root->list();
    {
        QFutureWatcher<QVector<Item::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(list_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto items = list_fut.result();
    ASSERT_EQ(1, items.size());

    // Create a file and check that it was created with correct type, name, and size 0.
    auto create_file_fut = root->create_file("file1", 0);
    {
        QFutureWatcher<Uploader::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(create_file_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto uploader = create_file_fut.result();
    EXPECT_EQ(0, uploader->size());
    auto finish_upload_fut = uploader->finish_upload();
    {
        QFutureWatcher<File::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(finish_upload_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto file = finish_upload_fut.result();
    EXPECT_EQ(ItemType::file, file->type());
    EXPECT_EQ("some_upload", file->name());
    EXPECT_EQ(0, file->size());
    EXPECT_EQ("some_id", file->native_identity());

    // For coverage: getting a file must return the correct one.
    auto get_fut = root->get("child_id");
    {
        QFutureWatcher<Item::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(get_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    file = dynamic_pointer_cast<File>(get_fut.result());
    EXPECT_EQ("child_id", file->native_identity());
    EXPECT_EQ("Child", file->name());

#if 0
    // Create a folder and check that it was created with correct type and name.
    auto create_folder_fut = root->create_folder("folder1");
    {
        QFutureWatcher<Folder::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(create_folder_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
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
    EXPECT_TRUE(root->equal_to(get_parent(file)));
    EXPECT_TRUE(root->equal_to(get_parent(folder)));
    EXPECT_EQ(root->native_identity(), file->parent_ids()[0]);
    EXPECT_EQ(root->native_identity(), folder->parent_ids()[0]);

    // Destroy the file and check that only the directory is left.
    file->delete_item().waitForFinished();
    items = root->list().result();
    ASSERT_EQ(1, items.size());
    folder = dynamic_pointer_cast<Folder>(items[0]);
    ASSERT_NE(nullptr, folder);
    EXPECT_EQ("folder1", folder->name());;

    // Destroy the folder and check that the root is empty.
    folder->delete_item().waitForFinished();
    items = root->list().result();
    ASSERT_EQ(0, items.size());
#endif
}

#if 0
TEST_F(FolderTest, nested)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    auto d1 = root->create_folder("d1").result();
    auto d2 = d1->create_folder("d2").result();

    // Parent of d2 must be d1.
    EXPECT_TRUE(get_parent(d2)->equal_to(d1));
    EXPECT_TRUE(d2->parent_ids()[0] == d1->native_identity());

    // Destroy is recursive
    d1->delete_item().waitForFinished();
    auto items = root->list().result();
    ASSERT_EQ(0, items.size());
}
#endif

TEST_F(FileTest, upload)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    // Get a file.
    auto lookup_fut = root->lookup("Child");
    {
        QFutureWatcher<QVector<Item::SPtr>> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(lookup_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto children = lookup_fut.result();
    ASSERT_EQ(1, children.size());
    auto file = dynamic_pointer_cast<File>(children[0]);
    EXPECT_EQ("child_id", file->native_identity());
    EXPECT_EQ("Child", file->name());

    auto create_uploader_fut = file->create_uploader(ConflictPolicy::error_if_conflict, 0);
    {
        QFutureWatcher<Uploader::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(create_uploader_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto uploader = create_uploader_fut.result();
    EXPECT_EQ(0, uploader->size());

    auto finish_upload_fut = uploader->finish_upload();
    {
        QFutureWatcher<File::SPtr> w;
        QSignalSpy spy(&w, &decltype(w)::finished);
        w.setFuture(finish_upload_fut);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
    }
    auto uploaded_file = finish_upload_fut.result();
    EXPECT_EQ("some_id", uploaded_file->native_identity());
    EXPECT_EQ("some_upload", uploaded_file->name());

#if 0
        QByteArray const contents = "Hello\n";
        auto written = uploader->socket()->write(contents);
        ASSERT_EQ(contents.size(), written);

        auto finish_upload_fut = uploader->finish_upload();
        {
            QFutureWatcher<File::SPtr> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_upload_fut);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        auto file = finish_upload_fut.result();
    }

    {
        // Don't upload anything.
        auto uploader = root->create_file("new_file").result();
        auto file = uploader->file();
        uploader->socket()->disconnectFromServer();

        // We never write anything, so there is no disconnected signal from the socket.

        auto state = uploader->finish_upload().result();
        EXPECT_EQ(TransferState::ok, state);
        ASSERT_EQ(0, uploader->file()->size());

        file->delete_item().waitForFinished();
    }

    {
        // Let the uploader go out of scope and check
        // that the file was created regardless.
        auto file = root->create_file("new_file").result()->file();
        ASSERT_EQ(0, file->size());

        file->delete_item().waitForFinished();
    }
#endif
}
#if 0

TEST_F(FileTest, create_uploader)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    auto file = root->create_file("new_file").result()->file();

    {
        auto uploader = file->create_uploader(ConflictPolicy::overwrite).result();

        auto finish_fut = uploader->finish_upload();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            // We never disconnected from the socket, so the transfer is still in progress.
            ASSERT_FALSE(spy.wait(SIGNAL_WAIT_TIME));
        }
        uploader->socket()->disconnectFromServer();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            // Now that we have disconnected, the future must become ready.
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        EXPECT_EQ(TransferState::ok, finish_fut.result());
    }

    // Same test again, but this time we write a bunch of data and don't disconnect.
    {
        auto uploader = file->create_uploader(ConflictPolicy::overwrite).result();

        std::string s(1000000, 'a');
        uploader->socket()->write(&s[0], s.size());

        auto finish_fut = uploader->finish_upload();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            // We never disconnected from the socket, so the transfer is still in progress.
            ASSERT_FALSE(spy.wait(SIGNAL_WAIT_TIME));
        }
        uploader->socket()->disconnectFromServer();
        {
            QFutureWatcher<TransferState> w;
            QSignalSpy spy(&w, &decltype(w)::finished);
            w.setFuture(finish_fut);
            // Now that we have disconnected, the future must become ready.
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
        }
        EXPECT_EQ(TransferState::ok, finish_fut.result());
    }

    file->delete_item().waitForFinished();
}

TEST_F(FileTest, cancel_upload)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    {
        // Upload a few bytes.
        auto uploader = root->create_file("new_file").result();

        // We haven't written anything and haven't pumped the event loop,
        // so the cancel is guaranteed to catch the uploader in the in_progress state.
        uploader->cancel();
        EXPECT_EQ(TransferState::cancelled, uploader->finish_upload().result());

        auto file = uploader->file();
        EXPECT_EQ(0, file->size());

        file->delete_item().waitForFinished();
    }

    {
        // Create a file with a few bytes.
        QByteArray original_contents = "Hello World!\n";
        write_file(root, "new_file", original_contents);
        auto file = dynamic_pointer_cast<File>(root->lookup("new_file").result());
        ASSERT_NE(nullptr, file);

        // Create an uploader for the file and write a bunch of bytes.
        auto uploader = file->create_uploader(ConflictPolicy::overwrite).result();
        QByteArray const contents(1024, 'a');
        auto written = uploader->socket()->write(contents);
        ASSERT_EQ(contents.size(), written);

        QSignalSpy spy(uploader->socket().get(), &QLocalSocket::bytesWritten);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

        // No disconnect here, so the transfer is still in progress. Now cancel.
        uploader->cancel().waitForFinished();

        // finish_upload() must indicate that the upload was cancelled.
        auto state = uploader->finish_upload().result();
        EXPECT_EQ(TransferState::cancelled, state);

        // The original file contents must still be intact.
        EXPECT_EQ(original_contents.size(), uploader->file()->size());
        ASSERT_TRUE(content_matches(uploader->file(), original_contents));

        file->delete_item().waitForFinished();
    }

    {
        // Upload a few bytes.
        auto uploader = root->create_file("new_file").result();
        auto file = uploader->file();
        QByteArray const contents = "Hello\n";

        // Finish the upload.
        auto written = uploader->socket()->write(contents);
        ASSERT_EQ(contents.size(), written);
        uploader->socket()->disconnectFromServer();

        // Pump the event loop for a bit, so the socket can finish doing its thing.
        QTimer timer;
        QSignalSpy spy(&timer, &QTimer::timeout);
        timer.start(SIGNAL_WAIT_TIME);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

        // Now send the cancel. The upload is finished already, and the cancel
        // is too late, so finish_upload() must report that the upload
        // worked OK.
        uploader->cancel();
        EXPECT_EQ(TransferState::ok, uploader->finish_upload().result());

        file->delete_item().waitForFinished();
    }
}

TEST_F(FileTest, upload_conflict)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    // Make a new file.
    auto uploader = root->create_file("new_file").result();
    auto file = uploader->file();

    // Write a few bytes.
    QByteArray const contents = "Hello\n";

    // Pump the event loop for a bit, so the socket can finish doing its thing.
    QTimer timer;
    QSignalSpy spy(&timer, &QTimer::timeout);
    timer.start(SIGNAL_WAIT_TIME);
    ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

    // Touch the file on disk to give it a new time stamp.
    ASSERT_EQ(0, system((string("touch ") + file->native_identity().toStdString()).c_str()));

    // Finish the upload.
    uploader->socket()->disconnectFromServer();

    try
    {
        // Must get an exception because the time stamps no longer match.
        uploader->finish_upload().result();
        FAIL();
    }
    catch (ConflictException const&)
    {
        // TODO: check exception details.
    }

    file->delete_item().waitForFinished();
}

TEST_F(FileTest, download)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    {
        // Download a few bytes.
        QByteArray const contents = "Hello\n";
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader = file->create_downloader().result();
        EXPECT_TRUE(file->equal_to(downloader->file()));

        auto socket = downloader->socket();
        QByteArray buf;
        do
        {
            // Need to pump the event loop while the socket does its thing.
            QSignalSpy spy(downloader->socket().get(), &QIODevice::readyRead);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
            auto bytes_to_read = socket->bytesAvailable();
            buf.append(socket->read(bytes_to_read));
        } while (buf.size() < contents.size());

        // Wait for disconnected signal.
        QSignalSpy spy(downloader->socket().get(), &QLocalSocket::disconnected);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

        auto state = downloader->finish_download().result();
        EXPECT_EQ(TransferState::ok, state);

        // Contents must match.
        EXPECT_EQ(contents, buf);
    }

    {
        // Download exactly 64 KB.
        QByteArray const contents(64 * 1024, 'a');
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader = file->create_downloader().result();
        EXPECT_TRUE(file->equal_to(downloader->file()));

        auto socket = downloader->socket();
        QByteArray buf;
        do
        {
            // Need to pump the event loop while the socket does its thing.
            QSignalSpy spy(downloader->socket().get(), &QIODevice::readyRead);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
            auto bytes_to_read = socket->bytesAvailable();
            buf.append(socket->read(bytes_to_read));
        } while (buf.size() < contents.size());

        // Wait for disconnected signal.
        QSignalSpy spy(downloader->socket().get(), &QLocalSocket::disconnected);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

        auto state = downloader->finish_download().result();
        EXPECT_EQ(TransferState::ok, state);

        // Contents must match
        EXPECT_EQ(contents, buf);
    }

    {
        // Download 1 MB + 1 bytes.
        QByteArray const contents(1024 * 1024 + 1, 'a');
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader = file->create_downloader().result();
        EXPECT_TRUE(file->equal_to(downloader->file()));

        auto socket = downloader->socket();
        QByteArray buf;
        do
        {
            // Need to pump the event loop while the socket does its thing.
            QSignalSpy spy(downloader->socket().get(), &QIODevice::readyRead);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
            auto bytes_to_read = socket->bytesAvailable();
            buf.append(socket->read(bytes_to_read));
        } while (buf.size() < contents.size());

        // Wait for disconnected signal.
        QSignalSpy spy(downloader->socket().get(), &QLocalSocket::disconnected);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

        auto state = downloader->finish_download().result();
        EXPECT_EQ(TransferState::ok, state);

        // Contents must match
        EXPECT_EQ(contents, buf);
    }

    {
        // Download file containing zero bytes
        QByteArray const contents;
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader = file->create_downloader().result();
        EXPECT_TRUE(file->equal_to(downloader->file()));

        auto socket = downloader->socket();

        // No readyRead every arrives in this case, just wait for disconnected.
        QSignalSpy spy(downloader->socket().get(), &QLocalSocket::disconnected);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

        auto state = downloader->finish_download().result();
        EXPECT_EQ(TransferState::ok, state);
    }

    {
        // Don't ever call read on empty file.
        QByteArray const contents;
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader = file->create_downloader().result();
        EXPECT_TRUE(file->equal_to(downloader->file()));

        // This succeeds because the provider disconnects as soon
        // as it realizes that there is nothing to write.
        downloader->finish_download().result();
    }

    {
        // Don't ever call read on non-empty file.
        QByteArray const contents("some contents");
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader = file->create_downloader().result();
        EXPECT_TRUE(file->equal_to(downloader->file()));

        try
        {
            downloader->finish_download().result();
            FAIL();
        }
        catch (StorageException const&)
        {
            // TODO: check exception details
        }
    }

    {
        // Let downloader go out of scope.
        QByteArray const contents("some contents");
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader = file->create_downloader().result();
    }

    {
        // Let downloader future go out of scope.
        QByteArray const contents("some contents");
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader_fut = file->create_downloader();
    }
}

TEST_F(FileTest, cancel_download)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    {
        // Download enough bytes to prevent a single read in the provider from completing the download.
        QByteArray const contents(1024 * 1024, 'a');
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        auto downloader = file->create_downloader().result();
        // We haven't read anything and haven't pumped the event loop,
        // so the cancel is guaranteed to catch the downloader in the in_progress state.
        downloader->cancel();
        EXPECT_EQ(TransferState::cancelled, downloader->finish_download().result());
    }

    {
        // Download a few bytes.
        QByteArray const contents = "Hello\n";
        write_file(root, "file", contents);

        auto item = root->lookup("file").result();
        File::SPtr file = dynamic_pointer_cast<File>(item);
        ASSERT_FALSE(file == nullptr);

        // Finish the download.
        auto downloader = file->create_downloader().result();
        auto socket = downloader->socket();
        QByteArray buf;
        do
        {
            // Need to pump the event loop while the socket does its thing.
            QSignalSpy spy(downloader->socket().get(), &QIODevice::readyRead);
            ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));
            auto bytes_to_read = socket->bytesAvailable();
            buf.append(socket->read(bytes_to_read));
        } while (buf.size() < contents.size());

        // Wait for disconnected signal.
        QSignalSpy spy(downloader->socket().get(), &QLocalSocket::disconnected);
        ASSERT_TRUE(spy.wait(SIGNAL_WAIT_TIME));

        // Now send the cancel. The download is finished already, and the cancel
        // is too late, so finish_download() must report that the download
        // worked OK.
        downloader->cancel();
        EXPECT_EQ(TransferState::ok, downloader->finish_download().result());
    }
}

Test_F(ItemTest, move)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    // Check that rename works within the same folder.
    auto f1 = root->create_file("f1").result()->file();
    auto f2 = f1->move(root, "f2").result();
    EXPECT_EQ("f2", f2->name());
    EXPECT_THROW(f1->name(), DestroyedException);  // TODO: check exception details.

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

Test_F(ItemTest, copy)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    QByteArray const contents = "hello\n";
    write_file(root, "file", contents);

    auto item = root->lookup("file").result();
    auto copied_item = item->copy(root, "copy_of_file").result();
    EXPECT_EQ("copy_of_file", copied_item->name());
    File::SPtr copied_file = dynamic_pointer_cast<File>(item);
    ASSERT_NE(nullptr, copied_file);
    EXPECT_TRUE(content_matches(copied_file, contents));
}

Test_F(ItemTest, recursive_copy)
{
    auto runtime = Runtime::create(connection());

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

Test_F(ItemTest, modified_time)
{
    auto runtime = Runtime::create(connection());

    auto acc = get_account(runtime);
    auto root = get_root(runtime);
    clear_folder(root);

    auto now = QDateTime::currentDateTimeUtc();
    // Need to sleep because time_t provides only 1-second resolution.
    sleep(1);
    auto file = root->create_file("file").result()->file();
    auto t = file->last_modified_time();
    // Rough check that the time is sane.
    EXPECT_LE(now, t);
    EXPECT_LE(t, now.addSecs(5));
}

Test_F(ItemTest, comparison)
{
    auto runtime = Runtime::create(connection());

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
#endif

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
