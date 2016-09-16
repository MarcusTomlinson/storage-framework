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

#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/internal/Handler.h>

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
                                 QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply)
    : status_(ItemJob::Loading)
    , method_(method)
{
    assert(!method.isEmpty());

    auto process_reply = [this](decltype(reply) const& r)
    {
        qDebug() << "ItemJobImpl process_reply callback";
        status_ = ItemJob::Finished;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    auto process_error = [this](StorageError const& error)
    {
        status_ = ItemJob::Error;
        error_ = error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<QList<storage::internal::ItemMetadata>>(this, reply, process_reply, process_error);
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
                                    QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply)
{
    unique_ptr<ItemJobImpl> impl(new ItemJobImpl(account, method, reply));
    auto job = new ItemJob(move(impl));
    job->p_->public_instance_ = job;
    return job;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
