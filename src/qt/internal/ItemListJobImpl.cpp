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

ItemListJobImpl::ItemListJobImpl(shared_ptr<AccountImpl> const& account,
                                 QString const& method,
                                 QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                                 std::function<bool(storage::internal::ItemMetadata const&)> const& validate)
    : status_(ItemListJob::Loading)
    , method_(method)
    , account_(account)
    , validate_(validate)
{
    assert(!method.isEmpty());
    assert(account);
    assert(validate);

    auto process_reply = [this](decltype(reply) const& r)
    {
        QList<Item> items;
        auto metadata = r.value();
        for (auto const& md : metadata)
        {
            if (!validate_(md))
            {
                continue;
            }
            try
            {
                auto item = ItemImpl::make_item(method_, md, account_);
                items.append(item);
            }
            catch (StorageError const& e)
            {
                // Bad metadata received from provider, make_item() has logged it.
            }
        }
        emit_items_ready(items);
        status_ = emit_status_changed(ItemListJob::Finished);
    };

    auto process_error = [this](StorageError const& error)
    {
        error_ = error;
        status_ = emit_status_changed(ItemListJob::Error);
    };

    new Handler<QList<storage::internal::ItemMetadata>>(this, reply, process_reply, process_error);
}

ItemListJobImpl::ItemListJobImpl(StorageError const& error)
    : status_(ItemListJob::Loading)
    , error_(error)
{
}

bool ItemListJobImpl::isValid() const
{
    return status_ != ItemListJob::Status::Error;
}

ItemListJob::Status ItemListJobImpl::status() const
{
    return status_;
}

StorageError ItemListJobImpl::error() const
{
    return error_;
}

ItemListJob* ItemListJobImpl::make_item_list_job(
                                shared_ptr<AccountImpl> const& account,
                                QString const& method,
                                QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                                std::function<bool(storage::internal::ItemMetadata const&)> const& validate)
{
    unique_ptr<ItemListJobImpl> impl(new ItemListJobImpl(account, method, reply, validate));
    auto job = new ItemListJob(move(impl));
    job->p_->public_instance_ = job;
    return job;
}

ItemListJob* ItemListJobImpl::make_item_list_job(StorageError const& error)
{
    unique_ptr<ItemListJobImpl> impl(new ItemListJobImpl(error));
    auto job = new ItemListJob(move(impl));
    job->p_->public_instance_ = job;
    job->p_->status_ = job->p_->emit_status_changed(ItemListJob::Error);
    return job;
}

ItemListJob::Status ItemListJobImpl::emit_status_changed(ItemListJob::Status new_status) const
{
    if (status_ == ItemListJob::Loading)  // Once in a final state, we don't emit the signal again.
    {
        // We defer emission of the signal so the client gets a chance to connect to the signal
        // in case we emit the signal from the constructor.
        QMetaObject::invokeMethod(public_instance_,
                                  "statusChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(unity::storage::qt::ItemListJob::Status, new_status));
    }
    return new_status;
}

void ItemListJobImpl::emit_items_ready(QList<Item> const& items) const
{
    QMetaObject::invokeMethod(public_instance_,
                              "itemsReady",
                              Qt::QueuedConnection,
                              Q_ARG(QList<unity::storage::qt::Item>, items));
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
