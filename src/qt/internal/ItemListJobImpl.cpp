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

#include <unity/storage/qt/internal/ItemListJobImpl.h>

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

ItemListJobImpl::ItemListJobImpl(shared_ptr<AccountImpl> const& account,
                                 QString const& method,
                                 QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                                 std::function<void(storage::internal::ItemMetadata const&)> const& validate)
    : ListJobImplBase(account, method, validate)
{
    auto process_reply = [this](decltype(reply)& r)
    {
        auto runtime = account_->runtime();
        if (!runtime || !runtime->isValid())
        {
            error_ = StorageErrorImpl::runtime_destroyed_error(method_ + ": Runtime was destroyed previously");
            status_ = ItemListJob::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

        QList<Item> items;
        auto metadata = r.value();
        for (auto const& md : metadata)
        {
            try
            {
                validate_(md);
                auto item = ItemImpl::make_item(method_, md, account_);
                items.append(item);
            }
            catch (StorageError const&)
            {
                // Bad metadata received from provider, validate_() or make_item() have logged it.
            }
        }
        status_ = ItemListJob::Finished;
        Q_EMIT public_instance_->itemsReady(items);
        Q_EMIT public_instance_->statusChanged(status_);
    };

    auto process_error = [this](StorageError const& error)
    {
        // TODO: method name is not being set this way.
        error_ = error;
        status_ = ItemListJob::Error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<QList<storage::internal::ItemMetadata>>(this, reply, process_reply, process_error);
}

ItemListJob* ItemListJobImpl::make_job(shared_ptr<AccountImpl> const& account,
                                       QString const& method,
                                       QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                                       std::function<void(storage::internal::ItemMetadata const&)> const& validate)
{
    unique_ptr<ItemListJobImpl> impl(new ItemListJobImpl(account, method, reply, validate));
    auto job = new ItemListJob(move(impl));
    job->p_->set_public_instance(job);
    return job;
}

ItemListJob* ItemListJobImpl::make_job(StorageError const& error)
{
    return ListJobImplBase::make_job(error);
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
