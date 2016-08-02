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

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/local_client/filesystem_exception.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace local_client
{

using namespace boost::filesystem;

void throw_filesystem_exception(QString const& method, filesystem_error const& e)
{
    QString msg = method + ": " + e.what();
    switch (e.code().value())
    {
        case EACCES:
        case EPERM:
        {
            throw PermissionException(msg);
        }
        case EDQUOT:
        case ENOSPC:
        {
            throw QuotaException(msg);  // Too messy to cover with a test case.  // LCOV_EXCL_LINE
        }
        default:
        {
            throw ResourceException(msg, e.code().value());
        }
    }
}

void throw_filesystem_exception(QString const& method, filesystem_error const& e, QString const& key)
{
    if (e.code().value() == ENOENT)
    {
        throw NotExistsException(method + ": " + e.what(), key);
    }
    throw_filesystem_exception(method, e);
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
