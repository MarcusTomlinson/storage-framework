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

#include "utils.h"

#include <unity/storage/internal/safe_strerror.h>
#include <unity/storage/provider/Exceptions.h>

#include <boost/algorithm/string.hpp>
#include <boost/exception/enable_current_exception.hpp>

#include <sys/stat.h>

using namespace unity::storage::provider;
using namespace std;

// Return modification time in nanoseconds since the epoch.

int64_t get_mtime_nsecs(string const& method, string const& path)
{
    using namespace unity::storage::internal;

    struct stat st;
    if (stat(path.c_str(), &st) == -1)
    {
        // LCOV_EXCL_START
        string msg = method + ": cannot stat \"" + path + "\": " + safe_strerror(errno);
        throw boost::enable_current_exception(ResourceException(msg, errno));
        // LCOV_EXCL_STOP
    }
    return int64_t(st.st_mtim.tv_sec) * 1000000000 + st.st_mtim.tv_nsec;
}

// Return true if the path uses the temp file prefix.

bool is_reserved_path(boost::filesystem::path const& path)
{
    string filename = path.filename().native();
    return boost::starts_with(filename, TMPFILE_PREFIX);
}

// Check that name is a valid file or directory name, that is, has a single component
// and is not "", ".", or "..". Also check that the name does not start with the
// temp file prefix. Throw if the name is invalid.

boost::filesystem::path sanitize(string const& method, string const& name)
{
    using namespace boost::filesystem;

    path p = name;
    if (!p.parent_path().empty())
    {
        // name contains more than one component.
        string msg = method + ": name \"" + name + "\" cannot contain a slash";
        throw boost::enable_current_exception(InvalidArgumentException(msg));
    }
    path filename = p.filename();
    if (filename.empty() || filename == "." || filename == "..")
    {
        // Not an allowable file name.
        string msg = method + ": invalid name: \"" + name + "\"";
        throw boost::enable_current_exception(InvalidArgumentException(msg));
    }
    if (is_reserved_path(filename))
    {
        string msg = string(method + ": names beginning with \"") + TMPFILE_PREFIX + "\" are reserved";
        throw boost::enable_current_exception(InvalidArgumentException(msg));
    }
    return p;
}

// Throw a StorageException that corresponds to a boost::filesystem_error.

void throw_storage_exception(string const& method, boost::filesystem::filesystem_error const& e)
{
    using namespace boost::system::errc;

    string msg = method + ": ";
    string path1 = e.path1().native();
    string path2 = e.path2().native();
    if (!path2.empty())
    {
        msg += "src = \"" + path1 + "\", target = \"" + path2 + "\"";
    }
    else
    {
        msg += "\"" + path1 + "\"";
    }
    msg += string(": ") + e.what();
    switch (e.code().value())
    {
        case permission_denied:
        case operation_not_permitted:
            throw boost::enable_current_exception(PermissionException(msg));
        case no_such_file_or_directory:
            throw boost::enable_current_exception(NotExistsException(msg, e.path1().native()));
        case file_exists:
            throw boost::enable_current_exception(
                    ExistsException(msg, e.path1().native(), e.path1().filename().native()));
        // LCOV_EXCL_START
        case no_space_on_device:
            throw boost::enable_current_exception(QuotaException(msg));
        default:
            throw boost::enable_current_exception(ResourceException(msg, e.code().value()));
        // LCOV_EXCL_STOP
    }
}

// Throw a storage exception that corresponds to a FileError.

void throw_storage_exception(string const& method, string const& msg, QFileDevice::FileError e)
{
    string const error_msg = method + ": " + msg;
    switch (e)
    {
        case QFileDevice::NoError:
            abort();  // Precondition violation
            break;
        case QFileDevice::PermissionsError:
            throw PermissionException(error_msg);
        default:
            throw ResourceException(error_msg + " (QIODevice error = " + to_string(e) + ")", e);
    }
}

// Throw a storage exception that corresponds to a LocalSocketError.

void throw_storage_exception(string const& method, string const& msg, QLocalSocket::LocalSocketError e)
{
    throw ResourceException(method + ": " + msg + " (QLocalSocket error = " + to_string(e) + ")", e);
}
