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

#include <unity/storage/qt/internal/ItemJobImpl.h>

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

ItemJobImpl::ItemJobImpl(shared_ptr<AccountImpl> const& account_impl,
                         QString const& method,
                         QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                         std::function<void(storage::internal::ItemMetadata const&)> const& validate)
    : status_(ItemJob::Status::Loading)
    , method_(method)
    , account_impl_(account_impl)
    , validate_(validate)
{
    assert(!method.isEmpty());
    assert(account_impl);
    assert(validate);

    auto process_reply = [this](decltype(reply) const& r)
    {
        auto runtime = account_impl_->runtime_impl();
        if (!runtime || !runtime->isValid())
        {
            error_ = StorageErrorImpl::runtime_destroyed_error(method_ + ": Runtime was destroyed previously");
            status_ = ItemJob::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

        auto metadata = r.value();
        try
        {
            validate_(metadata);
            item_ = ItemImpl::make_item(method_, metadata, account_impl_);
            status_ = ItemJob::Status::Finished;
        }
        catch (StorageError const& e)
        {
            // Bad metadata received from provider, validate_() or make_item() have logged it.
            // TODO: This does not set the method.
            error_ = e;
            status_ = ItemJob::Status::Error;
        }
        Q_EMIT public_instance_->statusChanged(status_);
    };

    auto process_error = [this](StorageError const& error)
    {
        error_ = error;
        status_ = ItemJob::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<storage::internal::ItemMetadata>(this, reply, process_reply, process_error);
}

ItemJobImpl::ItemJobImpl(shared_ptr<ItemImpl> const& item,
                         QString const& method,
                         QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                         std::function<void(storage::internal::ItemMetadata const&)> const& validate)
    : ItemJobImpl(item->account_impl(), method, reply, validate)
{
    item_impl_= item;
}

ItemJobImpl::ItemJobImpl(StorageError const& error)
    : status_(ItemJob::Status::Error)
    , error_(error)
{
}

bool ItemJobImpl::isValid() const
{
    return status_ != ItemJob::Status::Error;
}

ItemJob::Status ItemJobImpl::status() const
{
    return status_;
}

StorageError ItemJobImpl::error() const
{
    return error_;
}

Item ItemJobImpl::item() const
{
    return item_;
}

ItemJob* ItemJobImpl::make_job(shared_ptr<AccountImpl> const& account,
                               QString const& method,
                               QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                               std::function<void(storage::internal::ItemMetadata const&)> const& validate)
{
    unique_ptr<ItemJobImpl> impl(new ItemJobImpl(account, method, reply, validate));
    auto job = new ItemJob(move(impl));
    job->p_->public_instance_ = job;
    return job;
}

ItemJob* ItemJobImpl::make_job(shared_ptr<ItemImpl> const& item,
                               QString const& method,
                               QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                               std::function<void(storage::internal::ItemMetadata const&)> const& validate)
{
    unique_ptr<ItemJobImpl> impl(new ItemJobImpl(item, method, reply, validate));
    auto job = new ItemJob(move(impl));
    job->p_->public_instance_ = job;
    return job;
}

ItemJob* ItemJobImpl::make_job(StorageError const& error)
{
    unique_ptr<ItemJobImpl> impl(new ItemJobImpl(error));
    auto job = new ItemJob(move(impl));
    job->p_->public_instance_ = job;
    QMetaObject::invokeMethod(job,
                              "statusChanged",
                              Qt::QueuedConnection,
                              Q_ARG(unity::storage::qt::ItemJob::Status, job->p_->status_));
    return job;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
