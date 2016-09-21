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
#include <unity/storage/qt/internal/Handler.h>
#include <unity/storage/qt/internal/ItemImpl.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

ItemJobImpl::ItemJobImpl(shared_ptr<AccountImpl> const& account,
                         QString const& method,
                         QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                         std::function<void(storage::internal::ItemMetadata const&)> const& validate)
    : status_(ItemJob::Loading)
    , method_(method)
    , account_(account)
    , validate_(validate)
{
    assert(!method.isEmpty());
    assert(account);
    assert(validate);

    auto process_reply = [this](decltype(reply)& r)
    {
        auto metadata = r.value();
        try
        {
            validate_(metadata);
            item_ = ItemImpl::make_item(method_, metadata, account_);
            status_ = emit_status_changed(ItemJob::Finished);
        }
        catch (StorageError const& e)
        {
            // Bad metadata received from provider, validate_() or make_item() have logged it.
            error_ = e;
            status_ = emit_status_changed(ItemJob::Error);
        }
    };

    auto process_error = [this](StorageError const& error)
    {
        error_ = error;
        status_ = emit_status_changed(ItemJob::Error);
    };

    new Handler<storage::internal::ItemMetadata>(this, reply, process_reply, process_error);
}

ItemJobImpl::ItemJobImpl(StorageError const& error)
    : status_(ItemJob::Loading)
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
    return Item();  // TODO
}

ItemJob* ItemJobImpl::make_item_job(shared_ptr<AccountImpl> const& account,
                                    QString const& method,
                                    QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                                    std::function<void(storage::internal::ItemMetadata const&)> const& validate)
{
    unique_ptr<ItemJobImpl> impl(new ItemJobImpl(account, method, reply, validate));
    auto job = new ItemJob(move(impl));
    job->p_->public_instance_ = job;
    return job;
}

ItemJob* ItemJobImpl::make_item_job(StorageError const& error)
{
    unique_ptr<ItemJobImpl> impl(new ItemJobImpl(error));
    auto job = new ItemJob(move(impl));
    job->p_->public_instance_ = job;
    job->p_->status_ = job->p_->emit_status_changed(ItemJob::Error);
    return job;
}

ItemJob::Status ItemJobImpl::emit_status_changed(ItemJob::Status new_status) const
{
    // TODO: should be an assert!
    if (status_ == ItemJob::Loading)  // Once in a final state, we don't emit the signal again.
    {
        QMetaObject::invokeMethod(public_instance_,
                                  "statusChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(unity::storage::qt::ItemJob::Status, new_status));
    }
    return new_status;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
