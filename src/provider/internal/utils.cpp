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
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#include <unity/storage/provider/internal/utils.h>
#include <unity/storage/provider/Exceptions.h>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

boost::exception_ptr convert_exception_ptr(std::exception_ptr ep)
{
    // Convert std::exception_ptr to boost::exception_ptr
    try
    {
        std::rethrow_exception(ep);
    }
    catch (StorageException const& e)
    {
        return boost::copy_exception(e);
    }
    catch (...)
    {
        return boost::current_exception();
    }
}

}
}
}
}
