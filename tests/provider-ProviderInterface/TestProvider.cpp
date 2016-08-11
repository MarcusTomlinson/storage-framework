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

#include "TestProvider.h"
#include <unity/storage/provider/DownloadJob.h>
#include <unity/storage/provider/Exceptions.h>
#include <unity/storage/provider/UploadJob.h>

#include <QSocketNotifier>
#include <QTimer>
#include <unistd.h>
#include <algorithm>


using namespace std;
using namespace unity::storage;

class TestUploadJob : public UploadJob
{
public:
    TestUploadJob(std::string const& upload_id, Item const& item, int64_t size);
    boost::future<void> cancel() override;
    boost::future<Item> finish() override;

private:
    void read_some();

    Item const item_;
    int64_t const size_;
    QSocketNotifier notifier_;
    int64_t bytes_read_ = 0;
};

TestUploadJob::TestUploadJob(std::string const& upload_id, Item const& item,
                             int64_t size)
    : UploadJob(upload_id), item_(item), size_(size),
      notifier_(read_socket(), QSocketNotifier::Read)
{
    QObject::connect(&notifier_, &QSocketNotifier::activated,
                     [this]() { read_some(); });
    notifier_.setEnabled(true);
}

boost::future<void> TestUploadJob::cancel()
{
    boost::promise<void> p;
    notifier_.setEnabled(false);
    p.set_value();
    return p.get_future();
}

boost::future<Item> TestUploadJob::finish()
{
    boost::promise<Item> p;
    printf("TestUploadJob::finish(): %d read of expected %d\n", (int) bytes_read_, (int) size_);
    notifier_.setEnabled(false);
    if (bytes_read_ == size_)
    {
        p.set_value(item_);
    }
    else
    {
        p.set_exception(runtime_error("wrong number of bytes written"));
    }
    return p.get_future();
}

void TestUploadJob::read_some()
{
    printf("TestUploadJob::read_some(): %d read of expected %d\n", (int) bytes_read_, (int) size_);

    char buf[5];
    ssize_t n_read = read(read_socket(), buf, sizeof(buf));
    if (n_read < 0)
    {
        report_error(make_exception_ptr(runtime_error("Read failure")));
        notifier_.setEnabled(false);
    }
    else if (n_read == 0)
    {
        if (bytes_read_ != size_)
        {
            report_error(make_exception_ptr(runtime_error("wrong number of bytes")));
        }
        notifier_.setEnabled(false);
    }
    else
    {
        bytes_read_ += n_read;
        if (bytes_read_ > size_)
        {
            printf("Reporting error\n");
            report_error(make_exception_ptr(runtime_error("too many bytes written")));
            notifier_.setEnabled(false);
        }
    }
}

class TestDownloadJob : public DownloadJob
{
public:
    TestDownloadJob(std::string const& download_id, std::string const& data);
    boost::future<void> cancel() override;
    boost::future<void> finish() override;
private:
    void write_some();

    std::string const data_;
    ssize_t bytes_written_ = 0;
    QTimer timer_;
};

TestDownloadJob::TestDownloadJob(std::string const& download_id,
                                 std::string const& data)
    : DownloadJob(download_id), data_(data)
{
    timer_.setSingleShot(false);
    timer_.setInterval(10);
    QObject::connect(&timer_, &QTimer::timeout, [this]() { write_some(); });
    timer_.start();
}

boost::future<void> TestDownloadJob::cancel()
{
    timer_.stop();
    boost::promise<void> p;
    p.set_value();
    return p.get_future();
}

boost::future<void> TestDownloadJob::finish()
{
    boost::promise<void> p;
    if (bytes_written_ < (ssize_t)data_.size())
    {
        p.set_exception(runtime_error("Not all data read"));
    }
    else
    {
        p.set_value();
    }
    return p.get_future();
}

void TestDownloadJob::write_some()
{
    if (bytes_written_ >= (ssize_t)data_.size()) {
        report_complete();
        timer_.stop();
        return;
    }

    ssize_t n_written = write(write_socket(), data_.data() + bytes_written_,
                              min(data_.size() - bytes_written_, (size_t)2));
    if (n_written < 0)
    {
        report_error(make_exception_ptr(runtime_error("Write failure")));
        timer_.stop();
    }
    else
    {
        bytes_written_ += n_written;
    }
}


boost::future<ItemList> TestProvider::roots(Context const& ctx)
{
    Q_UNUSED(ctx);

    ItemList roots = {
        {"root_id", "", "Root", "etag", ItemType::root, {}},
    };
    boost::promise<ItemList> p;
    p.set_value(roots);
    return p.get_future();
}

boost::future<tuple<ItemList,string>> TestProvider::list(
    string const& item_id, string const& page_token, Context const& ctx)
{
    Q_UNUSED(ctx);

    boost::promise<tuple<ItemList,string>> p;

    if (item_id != "root_id")
    {
        p.set_exception(runtime_error("Unknown folder"));
    }
    else if (page_token == "")
    {
        ItemList children = {
            {"child1_id", "root_id", "Child 1", "etag", ItemType::file, {}},
            {"child2_id", "root_id", "Child 2", "etag", ItemType::file, {}},
        };
        p.set_value(make_tuple(children, "page_token"));
    }
    else if (page_token == "page_token")
    {
        ItemList children = {
            {"child3_id", "root_id", "Child 4", "etag", ItemType::file, {}},
            {"child4_id", "root_id", "Child 3", "etag", ItemType::file, {}},
        };
        p.set_value(make_tuple(children, ""));
    }
    else
    {
        p.set_exception(runtime_error("Unknown page token"));
    }
    return p.get_future();
}

boost::future<ItemList> TestProvider::lookup(
    string const& parent_id, string const& name, Context const& ctx)
{
    Q_UNUSED(ctx);

    boost::promise<ItemList> p;
    ItemList items = {
        {"child_id", parent_id, name, "etag", ItemType::file, {}},
    };
    p.set_value(items);
    return p.get_future();
}

boost::future<Item> TestProvider::metadata(
    string const& item_id, Context const& ctx)
{
    Q_UNUSED(ctx);

    boost::promise<Item> p;
    if (item_id == "root_id")
    {
        Item item = {"root_id", "", "Root", "etag", ItemType::root, {}};
        p.set_value(item);
    }
    else
    {
        p.set_exception(runtime_error("Unknown item"));
    }
    return p.get_future();
}

boost::future<Item> TestProvider::create_folder(
    string const& parent_id, string const& name, Context const& ctx)
{
    Q_UNUSED(ctx);

    boost::promise<Item> p;
    Item item = {"new_folder_id", parent_id, name, "etag", ItemType::folder, {}};
    p.set_value(item);
    return p.get_future();
}

boost::future<unique_ptr<UploadJob>> TestProvider::create_file(
    string const& parent_id, string const& name,
    int64_t size, string const& content_type, bool allow_overwrite,
    Context const& ctx)
{
    Q_UNUSED(content_type);
    Q_UNUSED(allow_overwrite);
    Q_UNUSED(ctx);

    boost::promise<unique_ptr<UploadJob>> p;
    Item item = {"new_file_id", parent_id, name, "etag", ItemType::file, {}};
    p.set_value(unique_ptr<UploadJob>(new TestUploadJob("upload_id", item, size)));
    return p.get_future();
}

boost::future<unique_ptr<UploadJob>> TestProvider::update(
    string const& item_id, int64_t size, string const& old_etag,
    Context const& ctx)
{
    Q_UNUSED(item_id);
    Q_UNUSED(old_etag);
    Q_UNUSED(ctx);

    boost::promise<unique_ptr<UploadJob>> p;
    Item item = {"item_id", "parent_id", "file name", "etag", ItemType::file, {}};
    p.set_value(unique_ptr<UploadJob>(new TestUploadJob("upload_id", item, size)));
    return p.get_future();
}

boost::future<unique_ptr<DownloadJob>> TestProvider::download(
    string const& item_id, Context const& ctx)
{
    Q_UNUSED(item_id);
    Q_UNUSED(ctx);

    boost::promise<unique_ptr<DownloadJob>> p;
    p.set_value(unique_ptr<DownloadJob>(
                    new TestDownloadJob("download_id", "Hello world")));
    return p.get_future();
}

boost::future<void> TestProvider::delete_item(
    string const& item_id, Context const& ctx)
{
    Q_UNUSED(ctx);

    boost::promise<void> p;
    if (item_id == "item_id")
    {
        p.set_value();
    }
    else
    {
        p.set_exception(runtime_error("Bad filename"));
    }
    return p.get_future();
}

boost::future<Item> TestProvider::move(
    string const& item_id, string const& new_parent_id,
    string const& new_name, Context const& ctx)
{
    Q_UNUSED(ctx);

    boost::promise<Item> p;
    Item item = {item_id, new_parent_id, new_name, "etag", ItemType::file, {}};
    p.set_value(item);
    return p.get_future();
}

boost::future<Item> TestProvider::copy(
    string const& item_id, string const& new_parent_id,
    string const& new_name, Context const& ctx)
{
    Q_UNUSED(item_id);
    Q_UNUSED(ctx);

    boost::promise<Item> p;
    Item item = {"new_id", new_parent_id, new_name, "etag", ItemType::file, {}};
    p.set_value(item);
    return p.get_future();
}
