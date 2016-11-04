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

#include <unity/storage/qt/internal/ListJobImplBase.h>

#include <unity/storage/internal/dbusmarshal.h>
#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/internal/AccountImpl.h>
#include <unity/storage/qt/internal/Handler.h>
#include <unity/storage/qt/internal/ItemImpl.h>
#include <unity/storage/qt/internal/RuntimeImpl.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

ListJobImplBase::ListJobImplBase()
    : status_(ItemListJob::Status::Finished)
{
}

ListJobImplBase::ListJobImplBase(shared_ptr<AccountImpl> const& account_impl,
                                 QString const& method,
                                 std::function<void(storage::internal::ItemMetadata const&)> const& validate)
    : status_(ItemListJob::Status::Loading)
    , method_(method)
    , account_impl_(account_impl)
    , validate_(validate)
{
    assert(!method.isEmpty());
    assert(account_impl);
    assert(validate);
}

ListJobImplBase::ListJobImplBase(StorageError const& error)
    : status_(ItemListJob::Status::Error)
    , error_(error)
{
}

bool ListJobImplBase::isValid() const
{
    return status_ != ItemListJob::Status::Error;
}

ItemListJob::Status ListJobImplBase::status() const
{
    return status_;
}

StorageError ListJobImplBase::error() const
{
    return error_;
}

void ListJobImplBase::set_public_instance(ItemListJob* p)
{
    assert(p);
    public_instance_ = p;
}

ItemListJob* ListJobImplBase::make_job(StorageError const& error)
{
    unique_ptr<ListJobImplBase> impl(new ListJobImplBase(error));
    auto job = new ItemListJob(move(impl));
    job->p_->public_instance_ = job;
    QMetaObject::invokeMethod(job,
                              "statusChanged",
                              Qt::QueuedConnection,
                              Q_ARG(unity::storage::qt::ItemListJob::Status, job->status()));
    return job;
}

ItemListJob* ListJobImplBase::make_empty_job()
{
    unique_ptr<ListJobImplBase> impl(new ListJobImplBase());
    auto job = new ItemListJob(move(impl));
    job->p_->public_instance_ = job;
    QMetaObject::invokeMethod(job,
                              "statusChanged",
                              Qt::QueuedConnection,
                              Q_ARG(unity::storage::qt::ItemListJob::Status, job->status()));
    return job;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
