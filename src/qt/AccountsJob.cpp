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

#include <unity/storage/qt/AccountsJob.h>

#include <unity/storage/qt/Account.h>
#include <unity/storage/qt/internal/AccountsJobImpl.h>

using namespace unity::storage::qt;
using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{

AccountsJob::AccountsJob(shared_ptr<internal::RuntimeImpl> const& runtime, QObject* parent)
    : QObject(parent)
    , p_(new internal::AccountsJobImpl(this, runtime))
{
}

AccountsJob::AccountsJob(StorageError const& error, QObject* parent)
    : QObject(parent)
    , p_(new internal::AccountsJobImpl(this, error))
{
}

AccountsJob::~AccountsJob() = default;

bool AccountsJob::isValid() const
{
    return p_->isValid();
}

AccountsJob::Status AccountsJob::status() const
{
    return p_->status();
}

StorageError AccountsJob::error() const
{
    return p_->error();
}

QList<Account> AccountsJob::accounts() const
{
    return p_->accounts();
}

}  // namespace qt
}  // namespace storage
}  // namespace unity
