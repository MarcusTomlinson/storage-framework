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

#include <unity/storage/qt/internal/MultiItemListJobImpl.h>

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

MultiItemListJobImpl::MultiItemListJobImpl(shared_ptr<ItemImpl> const& item,
                                           QString const& method,
                                           ReplyType const& reply,
                                           ValidateFunc const& validate,
                                           FetchFunc const& fetch_next)
    : ListJobImplBase(item->account_impl(), method, validate)
    , fetch_next_(fetch_next)
{
    assert(fetch_next);

    item_impl_ = item;

    process_reply_ = [this](ReplyType const& r)
    {
        if (status_ != ItemListJob::Status::Loading)
        {
            return;
        }
        auto runtime = item_impl_->account_impl()->runtime();
        if (!runtime || !runtime->isValid())
        {
            error_ = StorageErrorImpl::runtime_destroyed_error(method_ + ": Runtime was destroyed previously");
            status_ = ItemListJob::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

        QList<Item> items;
        auto metadata = r.argumentAt<0>();
        for (auto const& md : metadata)
        {
            try
            {
                validate_(md);
                auto item = ItemImpl::make_item(method_, md, account_);
                items.append(item);
            }
            catch (StorageError const& e)
            {
                // Bad metadata received from provider, validate_() or make_item() have logged it.
                error_ = e;
                status_ = ItemListJob::Status::Error;
                Q_EMIT public_instance_->statusChanged(status_);
                return;
            }
        }
        QString token = r.argumentAt<1>();
        if (token.isEmpty())
        {
            status_ = ItemListJob::Status::Finished;
        }
        if (!items.isEmpty())
        {
            Q_EMIT public_instance_->itemsReady(items);
        }
        if (token.isEmpty())
        {
            Q_EMIT public_instance_->statusChanged(status_);
        }
        else
        {
            new Handler<ReplyType>(this, fetch_next_(token), process_reply_, process_error_);
        }
    };

    process_error_ = [this](StorageError const& error)
    {
        assert(status_ == ItemListJob::Status::Loading);

        // TODO: method name is not being set this way.
        error_ = error;
        status_ = ItemListJob::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<ReplyType>(this, reply, process_reply_, process_error_);
}

ItemListJob* MultiItemListJobImpl::make_job(shared_ptr<ItemImpl> const& item,
                                            QString const& method,
                                            ReplyType const& reply,
                                            ValidateFunc const& validate,
                                            FetchFunc const& fetch_next)
{
    unique_ptr<MultiItemListJobImpl> impl(new MultiItemListJobImpl(item, method, reply, validate, fetch_next));
    auto job = new ItemListJob(move(impl));
    job->p_->set_public_instance(job);
    return job;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
