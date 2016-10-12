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

#include <unity/storage/qt/internal/MultiItemJobImpl.h>

#include <unity/storage/internal/dbusmarshal.h>
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

MultiItemJobImpl::MultiItemJobImpl(shared_ptr<AccountImpl> const& account_impl,
                                   QString const& method,
                                   ReplyType const& replies,
                                   ValidateFunc const& validate)
    : ListJobImplBase(account_impl, method, validate)
    , replies_remaining_(replies.size())
{
    assert(!method.isEmpty());
    assert(account_impl);
    assert(validate);

    // We ask the provider for the metadata for each of this item's parents.
    // As the replies trickle in, we track when the last reply has arrived and
    // signal that the job is complete.
    // If anything goes wrong at all, we report the first error and then ignore all
    // other replies.

    auto process_reply = [this](QDBusPendingReply<storage::internal::ItemMetadata> const& r)
    {
        if (status_ != ItemListJob::Status::Loading)
        {
            return;
        }

        --replies_remaining_;

        auto runtime = account_impl_->runtime_impl();
        if (!runtime || !runtime->isValid())
        {
            error_ = StorageErrorImpl::runtime_destroyed_error(method_ + ": Runtime was destroyed previously");
            status_ = ItemListJob::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

        auto metadata = r.value();
        Item item;
        try
        {
            validate_(metadata);
            item = ItemImpl::make_item(method_, metadata, account_impl_);
        }
        catch (StorageError const& e)
        {
            // Bad metadata received from provider, validate_() or make_item() have logged it.
            status_ = ItemListJob::Status::Error;
            error_ = e;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }
        QList<Item> items;
        items.append(item);
        if (replies_remaining_ == 0)
        {
            status_ = ItemListJob::Status::Finished;
        }
        Q_EMIT public_instance_->itemsReady(items);
        if (replies_remaining_ == 0)
        {
            Q_EMIT public_instance_->statusChanged(status_);
        }
    };

    auto process_error = [this](StorageError const& error)
    {
        if (status_ != ItemListJob::Status::Loading)
        {
            return;
        }
        // TODO: method name is not being set this way.
        error_ = error;
        status_ = ItemListJob::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    for (auto const& reply : replies)
    {
        new Handler<storage::internal::ItemMetadata>(this, reply, process_reply, process_error);
    }
}

ItemListJob* MultiItemJobImpl::make_job(shared_ptr<AccountImpl> const& account_impl,
                                        QString const& method,
                                        ReplyType const& replies,
                                        ValidateFunc const& validate)
{
    unique_ptr<MultiItemJobImpl> impl(new MultiItemJobImpl(account_impl, method, replies, validate));
    auto job = new ItemListJob(move(impl));
    job->p_->set_public_instance(job);
    return job;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
