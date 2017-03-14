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

#include "LocalProvider.h"

#include "LocalDownloadJob.h"
#include "LocalUploadJob.h"
#include "utils.h"

#include <unity/storage/provider/Exceptions.h>

#include <boost/algorithm/string.hpp>
#include <unity/util/GObjectMemory.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <gio/gio.h>
#include <glib.h>
#pragma GCC diagnostic pop

using namespace unity::storage::provider;
using namespace std;

namespace
{

// Return ${STORAGE_FRAMEWORK_ROOT}/storage-framework. If STORAGE_FRAMEWORK_ROOT
// is not set, return ${XDG_DATA_HOME}/storage-framework.
// ${STORAGE_FRAMEWORK_ROOT} or ${XDG_DATA_HOME} must exist and be a directory.
// If the storage-framework underneath that data directory does not exist, it is created.

string get_data_dir(string const& method)
{
    char const* dir = getenv("STORAGE_FRAMEWORK_ROOT");
    if (!dir || *dir == '\0')
    {
        dir = g_get_user_data_dir();  // Never fails.
    }

    boost::system::error_code ec;

    // The directory must exist.
    bool is_dir = boost::filesystem::is_directory(dir, ec);
    if (ec)
    {
        string msg = method + ": Cannot stat " + dir + ": " + ec.message();
        throw ResourceException(msg, errno);
    }
    if (!is_dir)
    {
        string msg = method + ": Environment variable STORAGE_FRAMEWORK_ROOT must denote a directory";
        throw InvalidArgumentException(msg);
    }

    // Create the storage-framework directory if it doesn't exist yet.
    string data_dir(dir);
    data_dir += "/storage-framework";
    if (!boost::filesystem::exists(data_dir))
    {
        boost::filesystem::create_directories(data_dir, ec);
        if (ec)
        {
            string msg = method + ": Cannot create " + dir + ": " + ec.message();
            throw ResourceException(msg, ec.value());
        }
    }
    return data_dir;
}

// Copy a file or directory (recursively). Ignore anything that has the temp file prefix
// or is not a file or directory.

void copy_recursively(boost::filesystem::path const& source, boost::filesystem::path const& target)
{
    using namespace boost::filesystem;

    if (is_reserved_path(source))
    {
        return;  // Don't copy temporary directories.
    }

    auto s = status(source);
    if (is_regular_file(s))
    {
        copy_file(source, target);
    }
    else if (is_directory(s))
    {
        copy_directory(source, target);  // Poorly named in boost; this creates the target dir without recursion
        for (directory_iterator it(source); it != directory_iterator(); ++it)
        {
            path source_entry = it->path();
            path target_entry = target;
            target_entry /= source_entry.filename();
            copy_recursively(source_entry, target_entry);
        }
    }
    else
    {
        // Ignore everything that's not a directory or file.
    }
}

// Convert nanoseconds since the epoch into ISO 8601 date-time.

string make_iso_date(int64_t nsecs_since_epoch)
{
    static char const* const FMT = "%Y-%m-%dT%TZ";  // ISO 8601, no fractional seconds.

    struct tm time;
    time_t secs_since_epoch = nsecs_since_epoch / 1000000000;
    gmtime_r(&secs_since_epoch, &time);

    char buf[128];
    strftime(buf, sizeof(buf), FMT, &time);
    return buf;
}

string get_content_type(string const& filename)
{
    using namespace unity::util;

    static string const unknown_content_type = "application/octet-stream";

    auto file = unique_gobject(g_file_new_for_path(filename.c_str()));
    assert(file);  // Cannot fail according to doc.

    GError* err = nullptr;
    auto full_info = unique_gobject(g_file_query_info(file.get(),
                                    G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE,
                                    G_FILE_QUERY_INFO_NONE,
                                    /* cancellable */ NULL,
                                    &err));
    if (!full_info)
    {
        return unknown_content_type;  // LCOV_EXCL_LINE
    }

    string content_type = g_file_info_get_attribute_string(full_info.get(), G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
    if (content_type.empty())
    {
        return unknown_content_type;  // LCOV_EXCL_LINE
    }
    return content_type;
}

// Simple wrapper template that deals with exception handling so we don't
// have to repeat ourselves endlessly in the various lambdas below.
// The auto deduction of the return type requires C++ 14.

template<typename F>
auto invoke_async(string const& method, F& functor)
{
    auto lambda = [method, functor]
    {
        try
        {
            return functor();
        }
        catch (StorageException const& e)
        {
            throw;
        }
        catch (boost::filesystem::filesystem_error const& e)
        {
            throw_storage_exception(method, e);
        }
        // LCOV_EXCL_START
        catch (std::exception const& e)
        {
            throw boost::enable_current_exception(UnknownException(e.what()));
        }
        // LCOV_EXCL_STOP
    };
    // TODO: boost::async is potentially expensive for some operations. Consider boost::asio thread pool?
    return boost::async(boost::launch::async, lambda);
}

}  // namespace

LocalProvider::LocalProvider()
    : root_(boost::filesystem::canonical(get_data_dir("LocalProvider()")))
{
}

LocalProvider::~LocalProvider() = default;

boost::future<ItemList> LocalProvider::roots(vector<string> const& /* keys */, Context const& /* context */)
{
    vector<Item> roots{ make_item("roots()", root_, status(root_)) };
    return boost::make_ready_future(roots);
}

boost::future<tuple<ItemList, string>> LocalProvider::list(string const& item_id,
                                                           string const& page_token,
                                                           vector<string> const& /* keys */,
                                                           Context const& /* context */)
{
    string const method = "list()";

    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    auto do_list = [This, method, item_id, page_token]
    {
        using namespace boost::filesystem;

        This->throw_if_not_valid(method, item_id);
        vector<Item> items;
        for (directory_iterator it(item_id); it != directory_iterator(); ++it)
        {
            auto dirent = *it;
            auto path = dirent.path();
            if (is_reserved_path(path))
            {
                continue;  // Hide temp files that we create during copy() and move().
            }
            Item i;
            try
            {
                auto st = dirent.status();
                i = This->make_item(method, path, st);
                items.push_back(i);
            }
            catch (std::exception const&)
            {
                // We ignore weird errors (such as entries that are not files or folders).
            }
        }
        return tuple<ItemList, string>(items, "");
    };

    return invoke_async(method, do_list);
}

boost::future<ItemList> LocalProvider::lookup(string const& parent_id,
                                              string const& name,
                                              vector<string> const& /* keys */,
                                              Context const& /* context */)
{
    string const method = "lookup()";

    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    auto do_lookup = [This, method, parent_id, name]
    {
        using namespace boost::filesystem;

        This->throw_if_not_valid(method, parent_id);
        auto sanitized_name = sanitize(method, name);
        path p = parent_id;
        p /= sanitized_name;
        auto st = status(p);
        vector<Item> items{ This->make_item(method, p, st) };
        return items;
    };

    return invoke_async(method, do_lookup);
}

boost::future<Item> LocalProvider::metadata(string const& item_id,
                                            vector<string> const& /* keys */,
                                            Context const& /* context */)
{
    string const method = "metadata()";

    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    auto do_metadata = [This, method, item_id]
    {
        using namespace boost::filesystem;

        This->throw_if_not_valid(method, item_id);
        path p = item_id;
        auto st = status(p);
        return This->make_item(method, p, st);
    };

    return invoke_async(method, do_metadata);
}

boost::future<Item> LocalProvider::create_folder(string const& parent_id,
                                                 string const& name,
                                                 vector<string> const& /* keys */,
                                                 Context const& /* context */)
{
    string const method = "create_folder()";

    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    auto do_create = [This, method, parent_id, name]
    {
        using namespace boost::filesystem;

        This->throw_if_not_valid(method, parent_id);
        auto sanitized_name = sanitize(method, name);
        path p = parent_id;
        p /= sanitized_name;
        // create_directory() succeeds if the directory exists already, so we need to check explicitly.
        if (exists(p))
        {
            string msg = method + ": \"" + p.native() + "\" exists already";
            throw boost::enable_current_exception(ExistsException(msg, p.native(), name));
        }
        create_directory(p);
        auto st = status(p);
        return This->make_item(method, p, st);
    };

    return invoke_async(method, do_create);
}

boost::future<unique_ptr<UploadJob>> LocalProvider::create_file(string const& parent_id,
                                                                string const& name,
                                                                int64_t size,
                                                                string const& /* content_type */,
                                                                bool allow_overwrite,
                                                                vector<string> const& /* keys */,
                                                                Context const& /* context */)
{
    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    boost::promise<unique_ptr<UploadJob>> p;
    string old_etag = allow_overwrite ? "" : "no_such_tag";
    p.set_value(unique_ptr<UploadJob>(new LocalUploadJob(This, parent_id, name, size, allow_overwrite)));
    return p.get_future();
}

boost::future<unique_ptr<UploadJob>> LocalProvider::update(string const& item_id,
                                                           int64_t size,
                                                           string const& old_etag,
                                                           vector<string> const& /* keys */,
                                                           Context const& /* context */)
{
    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    boost::promise<unique_ptr<UploadJob>> p;
    p.set_value(unique_ptr<UploadJob>(new LocalUploadJob(This, item_id, size, old_etag)));
    return p.get_future();
}

boost::future<unique_ptr<DownloadJob>> LocalProvider::download(string const& item_id,
                                                               string const& match_etag,
                                                               Context const& /* context */)
{
    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    boost::promise<unique_ptr<DownloadJob>> p;
    p.set_value(unique_ptr<DownloadJob>(new LocalDownloadJob(This, item_id, match_etag)));
    return p.get_future();
}

boost::future<void> LocalProvider::delete_item(string const& item_id, Context const& /* context */)
{
    string const method = "delete_item()";

    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    auto do_delete = [This, method, item_id]
    {
        using namespace boost::filesystem;

        This->throw_if_not_valid(method, item_id);
        if (canonical(item_id).native() == This->root_)
        {
            string msg = method + ": cannot delete root";
            throw boost::enable_current_exception(LogicException(msg));
        }
        remove_all(item_id);
    };

    return invoke_async(method, do_delete);
}

boost::future<Item> LocalProvider::move(string const& item_id,
                                        string const& new_parent_id,
                                        string const& new_name,
                                        vector<string> const& /* keys */,
                                        Context const& /* context */)
{
    string const method = "move()";

    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    auto do_move = [This, method, item_id, new_parent_id, new_name]
    {
        using namespace boost::filesystem;

        This->throw_if_not_valid(method, item_id);
        This->throw_if_not_valid(method, new_parent_id);
        auto sanitized_name = sanitize(method, new_name);

        path parent_path = new_parent_id;
        path target_path = parent_path / sanitized_name;

        if (exists(target_path))
        {
            string msg = method + ": \"" + target_path.native() + "\" exists already";
            throw boost::enable_current_exception(ExistsException(msg, target_path.native(), new_name));
        }

        // Small race condition here: if exists() just said that the target does not exist, it is
        // possible for it to have been created since. If so, if the target is a file or an empty
        // directory, it will be removed. In practice, this is unlikely to happen and, if it does,
        // it is not the end of the world.
        rename(item_id, target_path);
        auto st = status(target_path);
        return This->make_item(method, target_path, st);
    };

    return invoke_async(method, do_move);
}

boost::future<Item> LocalProvider::copy(string const& item_id,
                                        string const& new_parent_id,
                                        string const& new_name,
                                        vector<string> const& /* keys */,
                                        Context const& /* context */)
{
    string const method = "copy()";

    auto This = dynamic_pointer_cast<LocalProvider>(shared_from_this());
    auto do_copy = [This, method, item_id, new_parent_id, new_name]
    {
        using namespace boost::filesystem;

        This->throw_if_not_valid(method, item_id);
        This->throw_if_not_valid(method, new_parent_id);
        auto sanitized_name = sanitize(method, new_name);

        path parent_path = new_parent_id;
        path target_path = parent_path / sanitized_name;

        if (is_directory(item_id))
        {
            if (exists(target_path))
            {
                string msg = method + ": \"" + target_path.native() + "\" exists already";
                throw boost::enable_current_exception(ExistsException(msg, target_path.native(), new_name));
            }

            // For recursive copy, we create a temporary directory in lieu of target_path and recursively copy
            // everything into the temporary directory. This ensures that we don't invalidate directory iterators
            // by creating things while we are iterating, potentially getting trapped in an infinite loop.
            path tmp_path = canonical(parent_path);
            tmp_path /= unique_path(string(TMPFILE_PREFIX) + "-%%%%-%%%%-%%%%-%%%%");
            create_directories(tmp_path);
            for (directory_iterator it(item_id); it != directory_iterator(); ++it)
            {
                if (is_reserved_path(it->path()))
                {
                    continue;  // Don't recurse into the temporary directory
                }
                file_status s = it->status();
                if (is_directory(s) || is_regular_file(s))
                {
                    path source_entry = it->path();
                    path target_entry = tmp_path;
                    target_entry /= source_entry.filename();
                    copy_recursively(source_entry, target_entry);
                }
            }
            rename(tmp_path, target_path);
        }
        else
        {
            copy_file(item_id, target_path);
        }

        auto st = status(target_path);
        return This->make_item(method, target_path, st);
    };

    return invoke_async(method, do_copy);
}

// Make sure that id does not point outside the root.

void LocalProvider::throw_if_not_valid(string const& method, string const& id) const
{
    using namespace boost::filesystem;

    auto const root_id = root_.native();
    string suspect_id;
    try
    {
        suspect_id = canonical(id).native();
    }
    catch (filesystem_error const& e)
    {
        throw_storage_exception(method, e);
    }

    if (suspect_id != root_id && !boost::starts_with(suspect_id, root_id + "/"))
    {
        throw boost::enable_current_exception(InvalidArgumentException(method + ": invalid id: \"" + id + "\""));
    }
}

// Return an Item initialized from item_path and st.

Item LocalProvider::make_item(string const& method,
                              boost::filesystem::path const& item_path,
                              boost::filesystem::file_status const& st) const
{
    using namespace unity::storage;
    using namespace unity::storage::metadata;
    using namespace boost::filesystem;

    map<string, MetadataValue> meta;

    string const item_id = item_path.native();
    int64_t const mtime_nsecs = get_mtime_nsecs(method, item_id);
    string const iso_mtime = make_iso_date(mtime_nsecs);

    ItemType type;
    string name = item_path.filename().native();
    vector<string> parents{item_path.parent_path().native()};
    string etag;
    switch (st.type())
    {
        case regular_file:
            type = ItemType::file;
            etag = to_string(mtime_nsecs);
            meta.insert({SIZE_IN_BYTES, int64_t(file_size(item_path))});
            break;
        case directory_file:
            if (item_path == root_)
            {
                name = "/";
                parents.clear();
                type = ItemType::root;
            }
            else
            {
                type = ItemType::folder;
            }
            break;
        default:
            throw boost::enable_current_exception(
                    NotExistsException(method + ": \"" + item_id + "\" is neither a file nor a folder", item_id));
    }


    auto const info = space(item_path);
    meta.insert({FREE_SPACE_BYTES, int64_t(info.available)});
    meta.insert({USED_SPACE_BYTES, int64_t(info.capacity - info.available)});

    meta.insert({LAST_MODIFIED_TIME, iso_mtime});
    meta.insert({CONTENT_TYPE, get_content_type(item_id)});

    auto perms = st.permissions();
    bool writable;
    if (type == ItemType::file)
    {
        writable = perms & owner_write;
    }
    else
    {
        writable = perms & owner_write && perms & owner_exe;
    }
    meta.insert({WRITABLE, writable});

    return Item{ item_id, parents, name, etag, type, meta };
}
