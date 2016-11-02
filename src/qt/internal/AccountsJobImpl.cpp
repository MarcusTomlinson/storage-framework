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

static map<QString, QString> const BUS_NAMES =
{
    { "google-drive-scope", "com.canonical.StorageFramework.Provider.ProviderTest" },
    { "com.canonical.scopes.mcloud_mcloud_mcloud", "com.canonical.StorageFramework.Provider.McloudProvider" }
};

}  // namespace

AccountsJobImpl::AccountsJobImpl(AccountsJob* public_instance, shared_ptr<RuntimeImpl> const& runtime_impl)
    : public_instance_(public_instance)
    , status_(AccountsJob::Status::Loading)
    , runtime_impl_(runtime_impl)
{
    assert(public_instance);
    assert(runtime_impl);

    initialize_accounts();
}

AccountsJobImpl::AccountsJobImpl(AccountsJob* public_instance, StorageError const& error)
    : public_instance_(public_instance)
    , status_(AccountsJob::Status::Loading)
    , error_(error)
{
    assert(public_instance);
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

void AccountsJobImpl::manager_ready()
{
    timer_.stop();
    disconnect(this);
    initialize_accounts();
}

// LCOV_EXCL_START
void AccountsJobImpl::timeout()
{
    disconnect(this);
    error_ = StorageErrorImpl::local_comms_error("AccountsJob(): timeout retrieving Online accounts");
    status_ = emit_status_changed(AccountsJob::Status::Error);
}
// LCOV_EXCL_STOP

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
    }
    return runtime;
}

void AccountsJobImpl::initialize_accounts()
{
    auto runtime = get_runtime_impl("AccountsJob()");
    assert(runtime);

    auto manager = runtime->accounts_manager();
    if (!manager->isReady())
    {
        connect(manager.get(), &OnlineAccounts::Manager::ready, this, &AccountsJobImpl::manager_ready);
        connect(&timer_, &QTimer::timeout, this, &AccountsJobImpl::timeout);
        timer_.setSingleShot(true);
        timer_.start(30000);  // TODO: Need config for this eventually.
        return;
    }

    for (auto const map_entry : BUS_NAMES)
    {
        auto service_id = map_entry.first;
        for (auto const& a : manager->availableAccounts(service_id))
        {
            auto object_path = QStringLiteral("/provider/%1").arg(a->id());
            auto bus_name = map_entry.second;
            accounts_.append(AccountImpl::make_account(runtime,
                                                       bus_name,
                                                       object_path,
                                                       QString::number(a->id()),
                                                       a->serviceId(),
                                                       a->displayName()));
        }
    }
    status_ = emit_status_changed(AccountsJob::Status::Finished);
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
