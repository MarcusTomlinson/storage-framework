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

#include <unity/storage/qt/internal/AccountsJobImpl.h>

#include <unity/storage/qt/internal/RuntimeImpl.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

AccountsJobImpl::AccountsJobImpl(shared_ptr<RuntimeImpl> const& runtime)
    : is_valid_(true)
    , status_(AccountsJob::Loading)
    , runtime_(runtime)
{
    assert(runtime);
}

AccountsJobImpl::AccountsJobImpl(StorageError const& error)
    : is_valid_(false)
    , status_(AccountsJob::Error)
    , error_(error)
{
    assert(error.type() != StorageError::NoError);
}

bool AccountsJobImpl::isValid() const
{
    return is_valid_;
}

AccountsJob::Status AccountsJobImpl::status() const
{
    return status_;
}

StorageError AccountsJobImpl::error() const
{
    return error_;
}

QList<Account> AccountsJobImpl::accounts() const
{
    auto This = const_cast<AccountsJobImpl*>(this);
    auto runtime = runtime_.lock();
    if (!runtime)
    {
        This->is_valid_ = false;
        This->status_ = AccountsJob::Error;
        This->error_ = StorageErrorImpl::runtime_destroyed_error("AccountsJob::accounts()");
        return QList<Account>();
    }

    if (!is_valid_)
    {
        return QList<Account>();
    }
    
    return accounts_;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
