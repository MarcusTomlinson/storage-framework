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

#include "MockProvider.h"

#include <unity/storage/internal/metadata_keys.h>
#include <unity/storage/provider/Exceptions.h>

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

#include <chrono>
#include <thread>
#include <inttypes.h>

using namespace unity::storage;
using namespace unity::storage::provider;
using namespace std;

using boost::make_exceptional_future;
using boost::make_ready_future;

MockProvider::MockProvider()
{
}

MockProvider::MockProvider(string const& cmd)
    : cmd_(cmd)
{
}

boost::future<ItemList> MockProvider::roots(vector<string> const& keys, Context const&)
{
    if (cmd_ == "roots_slow")
    {
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (cmd_ == "not_a_root")
    {
        ItemList roots =
        {
            {"root_id", {}, "Root", "etag", ItemType::file, {}}
        };
        return make_ready_future<ItemList>(roots);
    }
    if (cmd_ == "roots_throw")
    {
        string msg = "roots(): I'm sorry Dave, I'm afraid I can't do that.";
        return make_exceptional_future<ItemList>(PermissionException(msg));
    }

    ItemList roots =
    {
        {"root_id", {}, "Root", "etag", ItemType::root, {}}
    };
    return make_ready_future<ItemList>(roots);
}

boost::future<tuple<ItemList,string>> MockProvider::list(
    string const& item_id, string const& page_token, vector<string> const& keys,
    Context const&)
{
    if (cmd_ == "list_slow")
    {
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (cmd_ == "list_empty")
    {
        boost::promise<tuple<ItemList,string>> p;
        p.set_value(make_tuple(ItemList(), string()));
        return p.get_future();
    }
    if (cmd_ == "list_no_permission")
    {
        string msg = string("permission denied");
        return make_exceptional_future<tuple<ItemList,string>>(PermissionException(msg));
    }
    if (cmd_ == "list_return_root")
    {
        ItemList children =
        {
            {
                "child_id", { "root_id" }, "Child", "etag", ItemType::root,
                { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
            }
        };
        boost::promise<tuple<ItemList,string>> p;
        p.set_value(make_tuple(children, string()));
        return p.get_future();
    }
    if (cmd_ == "list_two_children")
    {
        ItemList children;
        string next_token;
        if (page_token == "")
        {
            next_token = "next";
            children =
            {
                {
                    "child_id", { "root_id" }, "Child", "etag", ItemType::file,
                    { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
                }
            };
        }
        else
        {
            next_token = "";
            children =
            {
                {
                    "child2_id", { "root_id" }, "Child2", "etag", ItemType::file,
                    { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
                }
            };
        }
        boost::promise<tuple<ItemList,string>> p;
        p.set_value(make_tuple(children, next_token));
        return p.get_future();
    }
    if (item_id != "root_id")
    {
        string msg = string("Item::list(): no such item: \"") + item_id + "\"";
        return make_exceptional_future<tuple<ItemList,string>>(NotExistsException(msg, item_id));
    }
    if (page_token != "")
    {
        string msg = string("Item::list(): invalid page token: \"") + page_token + "\"";
        return make_exceptional_future<tuple<ItemList,string>>(LogicException("invalid page token"));
    }
    ItemList children =
    {
        {
            "child_id", { "root_id" }, "Child", "etag", ItemType::file,
            { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
        }
    };
    boost::promise<tuple<ItemList,string>> p;
    p.set_value(make_tuple(children, string()));
    return p.get_future();
}

boost::future<ItemList> MockProvider::lookup(
    string const& parent_id, string const& name, vector<string> const& keys, Context const&)
{
    if (parent_id != "root_id")
    {
        string msg = string("Folder::lookup(): no such item: \"") + parent_id + "\"";
        return make_exceptional_future<ItemList>(NotExistsException(msg, parent_id));
    }
    if (name != "Child")
    {
        string msg = string("Folder::lookup(): no such item: \"") + name + "\"";
        return make_exceptional_future<ItemList>(NotExistsException(msg, name));
    }
    ItemList children =
    {
        { "child_id", { "root_id" }, "Child", "etag", ItemType::file,
          { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } } }
    };
    return make_ready_future<ItemList>(children);
}

boost::future<Item> MockProvider::metadata(string const& item_id, vector<string> const& keys, Context const&)
{
    static int num_calls = 0;

    if (cmd_ == "slow_metadata")
    {
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (cmd_ == "empty_id")
    {
        Item metadata{"", {}, "Root", "etag", ItemType::root, {}};
        return make_ready_future<Item>(metadata);
    }
    if (cmd_== "two_parents_throw")
    {
        ++num_calls;
        switch (num_calls)
        {
            case 3:
                return make_exceptional_future<Item>(ResourceException("metadata(): weird error", 42));
            case 4:
                num_calls = 0;
                return make_exceptional_future<Item>(RemoteCommsException("metadata(): HTTP broken"));
            default:
                break;
        }
    }
    if (item_id == "root_id")
    {
        if (cmd_ == "bad_parent_metadata_from_child")
        {
            ++num_calls;
            if (num_calls == 2)
            {
                num_calls = 0;
                // On second call, we return type file for the root.
                Item metadata{"root_id", {}, "Root", "etag", ItemType::file, {}};
                return make_ready_future<Item>(metadata);
            }
        }
        if (cmd_ == "root_with_parent")
        {
            Item metadata{"root_id", { "this shouldn't be here" }, "Root", "etag", ItemType::root, {}};
            return make_ready_future<Item>(metadata);
        }
        Item metadata{"root_id", {}, "Root", "etag", ItemType::root, {}};
        return make_ready_future<Item>(metadata);
    }
    if (item_id == "child_id")
    {
        if (cmd_ == "no_parents")
        {
            Item metadata
            {
                "child_id", {}, "Child", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "empty_name")
        {
            Item metadata
            {
                "child_id", { "root_id" }, "", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "empty_etag")
        {
            Item metadata
            {
                "child_id", { "root_id" }, "Child", "", ItemType::file,
                { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "unknown_key")
        {
            Item metadata
            {
                "child_id", { "root_id" }, "Child", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, 0 },
                  { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" },
                  { metadata::DESCRIPTION, "child test file" },  // For coverage
                  { metadata::WRITABLE, true },                  // For coverage
                  { "unknown_key", "" }
                }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "missing_key")
        {
            Item metadata
            {
                "child_id", { "root_id" }, "Child", "etag", ItemType::file,
                { { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "wrong_type_for_time")
        {
            Item metadata
            {
                "child_id", { "root_id" }, "Child", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, 10 }, { metadata::LAST_MODIFIED_TIME, true } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "bad_parse_for_time")
        {
            Item metadata
            {
                "child_id", { "root_id" }, "Child", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, 10 }, { metadata::LAST_MODIFIED_TIME, "xyz" } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "missing_timezone")
        {
            Item metadata
            {
                "child_id", { "root_id" }, "Child", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30" } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "wrong_type_for_size")
        {
            Item metadata
            {
                "child_id", { "root_id" }, "Child", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, "10" }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "negative_size")
        {
            Item metadata
            {
                "child_id", { "root_id" }, "Child", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, -1 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "empty_parent")
        {
            Item metadata
            {
                "child_id", { "" }, "Child", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
            };
            return make_ready_future<Item>(metadata);
        }
        if (cmd_ == "two_parents" || cmd_ == "two_parents_throw")
        {
            Item metadata
            {
                "child_id", { "root_id", "child_folder_id" }, "Child", "etag", ItemType::file,
                { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
            };
            return make_ready_future<Item>(metadata);
        }
        Item metadata
        {
            "child_id", { "root_id" }, "Child", "etag", ItemType::file,
            { { metadata::SIZE_IN_BYTES, 10 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
        };
        return make_ready_future<Item>(metadata);
    }
    if (item_id == "child_folder_id")
    {
        Item metadata{"child_folder_id", { "root_id" }, "Child_Folder", "etag", ItemType::folder, {}};
        return make_ready_future<Item>(metadata);
    }
    return make_exceptional_future<Item>(NotExistsException("metadata(): no such item: " + item_id, item_id));
}

boost::future<Item> MockProvider::create_folder(
    string const& parent_id, string const& name, vector<string> const& keys,
    Context const&)
{
    if (cmd_ == "create_folder_returns_file")
    {
        Item metadata{"new_folder_id", { parent_id }, name, "etag", ItemType::file, {}};
        return make_ready_future<Item>(metadata);
    }
    Item metadata{"new_folder_id", { parent_id }, name, "etag", ItemType::folder, {}};
    return make_ready_future<Item>(metadata);
}

boost::future<unique_ptr<UploadJob>> MockProvider::create_file(
    string const&, string const&,
    int64_t, string const&, bool, vector<string> const&, Context const&)
{
    return make_ready_future<unique_ptr<UploadJob>>(new MockUploadJob(cmd_));
}

boost::future<unique_ptr<UploadJob>> MockProvider::update(
    string const&, int64_t, string const&, vector<string> const&, Context const&)
{
    if (cmd_ == "upload_slow")
    {
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (cmd_ == "upload_error")
    {
        unique_ptr<UploadJob> job(new MockUploadJob());
        ConflictException e("version mismatch");
        job->report_error(make_exception_ptr(e));
        return make_exceptional_future<unique_ptr<UploadJob>>(e);
    }
    return make_ready_future<unique_ptr<UploadJob>>(new MockUploadJob(cmd_));
}

boost::future<unique_ptr<DownloadJob>> MockProvider::download(
    string const&, string const&, Context const&)
{
    if (cmd_ == "download_slow")
    {
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (cmd_ == "download_error")
    {
        unique_ptr<DownloadJob> job(new MockDownloadJob());
        ResourceException e("test error", 42);
        job->report_error(make_exception_ptr(e));
        return make_exceptional_future<unique_ptr<DownloadJob>>(e);
    }
    unique_ptr<DownloadJob> job(new MockDownloadJob(cmd_));
    const char contents[] = "Hello world";
    if (write(job->write_socket(), contents, strlen(contents)) != int(strlen(contents)))
    {
        ResourceException e("download(): write failed", errno);
        job->report_error(make_exception_ptr(e));
        return make_exceptional_future<unique_ptr<DownloadJob>>(e);
    }
    if (cmd_ != "finish_download_error" && cmd_ != "finish_download_slow_error")
    {
        job->report_complete();
    }
    return make_ready_future(std::move(job));
}

boost::future<void> MockProvider::delete_item(
    string const& item_id, Context const&)
{
    if (cmd_ == "slow_delete")
    {
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (cmd_ == "delete_no_such_item")
    {
        string msg = "delete_item(): no such item: " + item_id;
        return make_exceptional_future<void>(NotExistsException(msg, item_id));
    }
    return make_ready_future();
}

boost::future<Item> MockProvider::move(
    string const& item_id, string const& new_parent_id,
    string const& new_name, vector<string> const& keys, Context const&)
{
    if (cmd_ == "move_returns_root")
    {
        Item metadata
        {
            "root_id", { new_parent_id }, new_name, "etag", ItemType::root,
            { { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
        };
        return make_ready_future(metadata);
    }
    if (cmd_ == "move_type_mismatch")
    {
        Item metadata
        {
            item_id, { new_parent_id }, new_name, "etag", ItemType::folder,
            { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
        };
        return make_ready_future(metadata);
    }
    Item metadata
    {
        item_id, { new_parent_id }, new_name, "etag", ItemType::file,
        { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
    };
    return make_ready_future(metadata);
}

boost::future<Item> MockProvider::copy(
    string const&, string const& new_parent_id,
    string const& new_name, vector<string> const& keys, Context const&)
{
    if (cmd_ == "copy_type_mismatch")
    {
        Item metadata
        {
            "new_item_id", { new_parent_id }, new_name, "etag", ItemType::folder,
            { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
        };
        return make_ready_future(metadata);
    }
    Item metadata
    {
        "new_item_id", { new_parent_id }, new_name, "etag", ItemType::file,
        { { metadata::SIZE_IN_BYTES, 0 }, { metadata::LAST_MODIFIED_TIME, "2007-04-05T14:30Z" } }
    };
    return make_ready_future(metadata);
}

MockUploadJob::MockUploadJob()
    : UploadJob("some_id")
{
}

MockUploadJob::MockUploadJob(string const& cmd)
    : UploadJob("some_id")
    , cmd_(cmd)
{
}

boost::future<void> MockUploadJob::cancel()
{
    return make_ready_future();
}

boost::future<Item> MockUploadJob::finish()
{
    if (cmd_ == "finish_upload_slow" || cmd_ == "finish_upload_slow_error")
    {
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (cmd_ == "finish_upload_error" || cmd_ == "finish_upload_slow_error")
    {
        return make_exceptional_future<Item>(ResourceException("out of memory", 99));
    }
    if (cmd_ == "create_file_exists")
    {
        ExistsException e("file exists", "child_id", "Child");
        return make_exceptional_future<Item>(e);
    }
    if (cmd_ == "upload_returns_dir")
    {
        Item metadata{"some_id", { "root_id" }, "some_upload", "etag", ItemType::folder, {}};
        return make_ready_future<Item>(metadata);
    }
    Item metadata
    {
        "child_id", { "root_id" }, "some_upload", "etag", ItemType::file,
        { { metadata::SIZE_IN_BYTES, 10 }, { metadata::LAST_MODIFIED_TIME, "2011-04-05T14:30:10.005Z" } }
    };
    return make_ready_future(metadata);
}

MockDownloadJob::MockDownloadJob()
    : DownloadJob("some_id")
{
}

MockDownloadJob::MockDownloadJob(string const& cmd)
    : DownloadJob("some_id")
    , cmd_(cmd)
{
}

boost::future<void> MockDownloadJob::cancel()
{
    return make_ready_future();
}

boost::future<void> MockDownloadJob::finish()
{
    if (cmd_ == "finish_download_slow" || cmd_ == "finish_download_slow_error")
    {
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (cmd_ == "finish_download_error" || cmd_ == "finish_download_slow_error")
    {
        return make_exceptional_future<void>(NotExistsException("no such item", "item_id"));
    }
    return make_ready_future();
}
