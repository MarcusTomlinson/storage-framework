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

#include <unity/storage/qt/client/internal/remote_client/RuntimeImpl.h>

#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/internal/make_future.h>
#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>
#include <unity/storage/qt/client/internal/remote_client/dbusmarshal.h>

#include <QDBusMetaType>

// TODO: Hack until we can use the registry instead
#include <OnlineAccounts/Account>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <glib.h>
#pragma GCC diagnostic pop

#include <cassert>
#include <cstdlib>

#include <unistd.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{

RuntimeImpl::RuntimeImpl()
    : conn_(QDBusConnection::sessionBus())
{
    if (!conn_.isConnected())
    {
        throw LocalCommsException("Runtime: cannot connect to session bus");  // LCOV_EXCL_LINE
    }
    qDBusRegisterMetaType<unity::storage::internal::ItemMetadata>();
    qDBusRegisterMetaType<QList<unity::storage::internal::ItemMetadata>>();
}

RuntimeImpl::~RuntimeImpl()
{
    try
    {
        shutdown();
    }
    // LCOV_EXCL_START
    catch (std::exception const&)
    {
        qCritical() << "shutdown error";  // TODO, log the error properly
    }
    // LCOV_EXCL_STOP
}

void RuntimeImpl::shutdown()
{
    if (destroyed_)
    {
        return;
    }
    destroyed_ = true;
    conn_.disconnectFromBus(conn_.name());
}

QFuture<QVector<Account::SPtr>> RuntimeImpl::accounts()
{
    if (destroyed_)
    {
        return make_exceptional_future(qf_, RuntimeDestroyedException("Runtime::accounts()"));
    }

    if (!manager_)
    {
        manager_.reset(new OnlineAccounts::Manager(""));
        connect(manager_.get(), &OnlineAccounts::Manager::ready, this, &RuntimeImpl::manager_ready);
        connect(&timer_, &QTimer::timeout, this, &RuntimeImpl::timeout);
        timer_.setSingleShot(true);
        timer_.start(5000);
    }

    qf_.reportStarted();
    return qf_.future();
}

QDBusConnection& RuntimeImpl::connection()
{
    return conn_;
}

void RuntimeImpl::manager_ready()
{
    if (destroyed_)
    {
        make_exceptional_future(qf_, RuntimeDestroyedException("Runtime::accounts()"));
        return;
    }

    timer_.stop();
    try
    {
        QVector<Account::SPtr> accounts;
        for (auto const& a : manager_->availableAccounts("google-drive-scope"))
        {
            qDebug() << "got account:" << a->displayName() << a->serviceId() << a->id();
            auto impl = new AccountImpl(public_instance_, a->id(), "", a->serviceId(), a->displayName());
            Account::SPtr acc(new Account(impl));
            impl->set_public_instance(acc);
            accounts.append(acc);
        }
        accounts_ = accounts;
        make_ready_future(qf_, accounts);
    }
    catch (StorageException const& e)
    {
        make_exceptional_future(qf_, e);
    }
}

void RuntimeImpl::timeout()
{
    make_exceptional_future(qf_, ResourceException("Runtime::accounts(): timeout retrieving Online accounts", 0));
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
