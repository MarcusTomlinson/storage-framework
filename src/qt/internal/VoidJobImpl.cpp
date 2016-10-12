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

#include <unity/storage/qt/internal/VoidJobImpl.h>

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

VoidJobImpl::VoidJobImpl(shared_ptr<ItemImpl> const& item_impl,
                         QString const& method,
                         QDBusPendingReply<void>& reply)
    : status_(VoidJob::Status::Loading)
    , method_(method)
    , item_impl_(item_impl)
{
    assert(!method_.isEmpty());
    assert(item_impl);

    auto process_reply = [this](decltype(reply)&)
    {
        auto runtime = item_impl_->runtime_impl();
        if (!runtime || !runtime->isValid())
        {
            error_ = StorageErrorImpl::runtime_destroyed_error(method_ + ": Runtime was destroyed previously");
            status_ = VoidJob::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

        status_ = VoidJob::Status::Finished;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    auto process_error = [this](StorageError const& error)
    {
        error_ = error;
        status_ = VoidJob::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<void>(this, reply, process_reply, process_error);
}

VoidJobImpl::VoidJobImpl(StorageError const& error)
    : status_(VoidJob::Status::Error)
    , error_(error)
{
}

bool VoidJobImpl::isValid() const
{
    return status_ != VoidJob::Status::Error;
}

VoidJob::Status VoidJobImpl::status() const
{
    return status_;
}

StorageError VoidJobImpl::error() const
{
    return error_;
}

VoidJob* VoidJobImpl::make_job(shared_ptr<ItemImpl> const& item,
                               QString const& method,
                               QDBusPendingReply<void>& reply)
{
    unique_ptr<VoidJobImpl> impl(new VoidJobImpl(item, method, reply));
    auto job = new VoidJob(move(impl));
    job->p_->public_instance_ = job;
    return job;
}

VoidJob* VoidJobImpl::make_job(StorageError const& error)
{
    unique_ptr<VoidJobImpl> impl(new VoidJobImpl(error));
    auto job = new VoidJob(move(impl));
    job->p_->public_instance_ = job;
    QMetaObject::invokeMethod(job,
                              "statusChanged",
                              Qt::QueuedConnection,
                              Q_ARG(unity::storage::qt::VoidJob::Status, job->p_->status_));
    return job;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
