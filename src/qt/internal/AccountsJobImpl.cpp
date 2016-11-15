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

#include <unity/storage/qt/internal/AccountsJobImpl.h>

#include "RegistryInterface.h"
#include <unity/storage/qt/internal/AccountImpl.h>
#include <unity/storage/qt/internal/Handler.h>
#include <unity/storage/qt/internal/RuntimeImpl.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

AccountsJobImpl::AccountsJobImpl(shared_ptr<RuntimeImpl> const& runtime_impl,
                                 QString const& method,
                                 QDBusPendingReply<QList<storage::internal::AccountDetails>>& reply)
    : status_(AccountsJob::Status::Loading)
    , runtime_impl_(runtime_impl)
{
    assert(runtime_impl);

    auto process_reply = [this, method](decltype(reply)& r)
    {
        auto runtime = get_runtime_impl(method);
        if (!runtime || !runtime->isValid())
        {
            return;
        }

        for (auto const& ad : r.value())
        {
            auto a = AccountImpl::make_account(runtime, ad);
            accounts_.append(a);
        }
        status_ = AccountsJob::Status::Finished;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    auto process_error = [this](StorageError const& error)
    {
        error_ = error;
        status_ = AccountsJob::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<QList<storage::internal::AccountDetails>>(this, reply, process_reply, process_error);
}

AccountsJobImpl::AccountsJobImpl(StorageError const& error)
    : status_(AccountsJob::Status::Error)
    , error_(error)
{
    assert(error.type() != StorageError::Type::NoError);

    status_ = emit_status_changed(AccountsJob::Status::Error);
}

bool AccountsJobImpl::isValid() const
{
    return status_ != AccountsJob::Status::Error;
}

AccountsJob::Status AccountsJobImpl::status() const
{
    return status_;
}

StorageError AccountsJobImpl::error() const
{
    return error_;
}

QList<Account> AccountsJobImpl::accounts() const
{
    auto runtime = get_runtime_impl("AccountsJob::accounts()");
    if (!runtime)
    {
        return QList<Account>();
    }
    if (status_ != AccountsJob::Status::Finished)
    {
        return QList<Account>();
    }
    return accounts_;
}

QVariantList AccountsJobImpl::accountsAsVariantList() const
{
    QVariantList account_list;
    for (auto const& a : accounts())
    {
        account_list.append(QVariant::fromValue(a));
    }
    return account_list;
}

AccountsJob* AccountsJobImpl::make_job(shared_ptr<RuntimeImpl> const& runtime,
                                       QString const& method,
                                       QDBusPendingReply<QList<storage::internal::AccountDetails>>& reply)
{
    unique_ptr<AccountsJobImpl> impl(new AccountsJobImpl(runtime, method, reply));
    auto job = new AccountsJob(move(impl));
    job->p_->public_instance_ = job;
    return job;
}

AccountsJob* AccountsJobImpl::make_job(StorageError const& error)
{
    unique_ptr<AccountsJobImpl> impl(new AccountsJobImpl(error));
    auto job = new AccountsJob(move(impl));
    job->p_->public_instance_ = job;
    QMetaObject::invokeMethod(job,
                              "statusChanged",
                              Qt::QueuedConnection,
                              Q_ARG(unity::storage::qt::AccountsJob::Status, job->p_->status_));
    return job;
}

AccountsJob::Status AccountsJobImpl::emit_status_changed(AccountsJob::Status new_status) const
{
    if (status_ == AccountsJob::Status::Loading)  // Once in a final state, we don't emit the signal again.
    {
        // We defer emission of the signal so the client gets a chance to connect to the signal
        // in case we emit the signal from the constructor.
        QMetaObject::invokeMethod(public_instance_,
                                  "statusChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(unity::storage::qt::AccountsJob::Status, new_status));
    }
    return new_status;
}

shared_ptr<RuntimeImpl> AccountsJobImpl::get_runtime_impl(QString const& method) const
{
    auto runtime = runtime_impl_.lock();
    if (!runtime || !runtime->isValid())
    {
        QString msg = method + ": Runtime was destroyed previously";
        auto This = const_cast<AccountsJobImpl*>(this);
        This->error_ = StorageErrorImpl::runtime_destroyed_error(msg);
        This->status_ = emit_status_changed(AccountsJob::Status::Error);
        return nullptr;
    }
    return runtime;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
