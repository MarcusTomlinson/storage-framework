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

#include <unity/storage/qt/internal/AccountImpl.h>
#include <unity/storage/qt/internal/RuntimeImpl.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>

#include <OnlineAccounts/Account>

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

namespace
{

// TODO: We retrieve the accounts directly from online accounts until we have a working registry.

static const map<QString, QString> BUS_NAMES =
{
    { "google-drive-scope", "com.canonical.StorageFramework.Provider.ProviderTest" },
    { "com.canonical.scopes.mcloud_mcloud_mcloud", "com.canonical.StorageFramework.Provider.McloudProvider" }
};

}  // namespace

AccountsJobImpl::AccountsJobImpl(AccountsJob* public_instance, shared_ptr<RuntimeImpl> const& runtime)
    : public_instance_(public_instance)
    , status_(AccountsJob::Loading)
    , runtime_(runtime)
{
    assert(public_instance);
    assert(runtime);

    initialize_accounts();
}

AccountsJobImpl::AccountsJobImpl(AccountsJob* public_instance, StorageError const& error)
    : public_instance_(public_instance)
    , status_(AccountsJob::Error)
    , error_(error)
{
    assert(public_instance);
    assert(error.type() != StorageError::NoError);

    emit_status_changed();
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
    auto runtime = get_runtime("AccountsJob::accounts()");
    if (!runtime)
    {
        return QList<Account>();
    }
    if (!isValid())
    {
        return QList<Account>();
    }
    return accounts_;
}

void AccountsJobImpl::manager_ready()
{
    timer_.stop();
    disconnect(this);
    initialize_accounts();
}

void AccountsJobImpl::timeout()
{
    disconnect(this);
    status_ = AccountsJob::Error;
    error_ = StorageErrorImpl::local_comms_error("AccountsJob(): timeout retrieving Online accounts");
    emit_status_changed();
}

shared_ptr<RuntimeImpl> AccountsJobImpl::get_runtime(QString const& method) const
{
    auto runtime = runtime_.lock();
    if (!runtime)
    {
        auto This = const_cast<AccountsJobImpl*>(this);
        This->status_ = AccountsJob::Error;
        This->error_ = StorageErrorImpl::runtime_destroyed_error(method);
        emit_status_changed();
    }
    return runtime;
}

void AccountsJobImpl::initialize_accounts()
{
    auto runtime = get_runtime("AccountsJob()");
    if (!runtime)
    {
        return;
    }

    auto manager = runtime->accounts_manager();
    if (!manager->isReady())
    {
        connect(manager.get(), &OnlineAccounts::Manager::ready, this, &AccountsJobImpl::manager_ready);
        connect(&timer_, &QTimer::timeout, this, &AccountsJobImpl::timeout);
        timer_.setSingleShot(true);
        timer_.start(30000);
        return;
    }

    for (auto const map_entry : BUS_NAMES)
    {
        auto service_id = map_entry.first;
        for (auto const& a : manager->availableAccounts(service_id))
        {
            auto object_path = QStringLiteral("/provider/%1").arg(a->id());
            auto bus_name = map_entry.second;
            accounts_.append(AccountImpl::make_account(bus_name, object_path, a->serviceId(), "", a->displayName()));
        }
    }
    status_ = AccountsJob::Status::Finished;
    emit_status_changed();
}

void AccountsJobImpl::emit_status_changed() const
{
    // Defer emitting the status changed signal because we may need to emit the signal from the constructor.
    if (status_ != AccountsJob::Status::Error)
    {
        QMetaObject::invokeMethod(public_instance_,
                                  "statusChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(AccountsJob::Status, status_));
    }
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
