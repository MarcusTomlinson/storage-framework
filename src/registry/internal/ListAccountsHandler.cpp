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

#include <unity/storage/registry/internal/ListAccountsHandler.h>

#include <unity/storage/internal/AccountDetails.h>
#include <unity/storage/registry/internal/qdbus-last-error-msg.h>

#include <OnlineAccounts/Account>
#include <QDebug>

using namespace std;

namespace unity
{
namespace storage
{
namespace registry
{
namespace internal
{

ListAccountsHandler::ListAccountsHandler(QDBusConnection const& conn,
                                         QDBusMessage const& msg,
                                         shared_ptr<storage::internal::InactivityTimer> const& timer)
    : conn_(conn)
    , msg_(msg)
    , manager_("", conn)
    , activity_notifier_(timer)
{
    connect(&manager_, &OnlineAccounts::Manager::ready, this, &ListAccountsHandler::manager_ready);
    connect(&timer_, &QTimer::timeout, this, &ListAccountsHandler::timeout);
    timer_.setSingleShot(true);
    timer_.start(25000);  // TODO: Need config for this eventually.
}

ListAccountsHandler::~ListAccountsHandler() = default;

namespace
{

// TODO: This is a hack until Online Accounts is updated to give us the provider ID, provider name, and icon name.

struct ProviderDetails
{
    char const* bus_name;
    char const* provider_name;
};

static map<QString, ProviderDetails> const BUS_NAMES =
{
    { "storage-provider-test", { "com.canonical.StorageFramework.Provider.ProviderTest", "Test Provider" } },
    { "storage-provider-mcloud", { "com.canonical.StorageFramework.Provider.McloudProvider", "mcloud" } },
    { "storage-provider-owncloud", { "com.canonical.StorageFramework.Provider.OwnCloud", "ownCloud" } },
    { "storage-provider-onedrive", { "com.canonical.StorageFramework.Provider.OnedriveProvider", "OneDrive" } },
    { "storage-provider-gdrive", { "com.canonical.StorageFramework.Provider.GdriveProvider", "GDrive" } },
};

}  // namespace

void ListAccountsHandler::manager_ready()
{
    timer_.stop();
    disconnect(this);
    deleteLater();

    QList<storage::internal::AccountDetails> accounts;
    for (auto const& acct : manager_.availableAccounts())
    {
        auto const it = BUS_NAMES.find(acct->serviceId());
        if (it == BUS_NAMES.end())
        {
            continue;
        }

        storage::internal::AccountDetails ad;
        ad.busName = it->second.bus_name;
        ad.objectPath = QDBusObjectPath(QStringLiteral("/provider/%1").arg(acct->id()));
        ad.id = acct->id();
        ad.serviceId = acct->serviceId();
        ad.displayName = acct->displayName();
        ad.providerName = it->second.provider_name;
        ad.iconName = "";

        accounts.append(ad);
    }

    // Add an entry for the local provider, which Online Accounts doesn't know about.
    storage::internal::AccountDetails ad;
    ad.busName = "com.canonical.StorageFramework.Provider.Local";
    ad.objectPath = QDBusObjectPath(QStringLiteral("/provider/"));
    ad.id = numeric_limits<decltype(ad.id)>::max();
    ad.serviceId = "";
    ad.displayName = "Local Provider";
    ad.providerName = "Local Provider";
    ad.iconName = "";
    accounts.append(ad);

    if (!conn_.send(msg_.createReply(QVariant::fromValue(accounts))))
    {
        auto msg = last_error_msg(conn_);
        qCritical().noquote() << "ListAccounts(): could not send DBus reply" + msg;
    }
}

void ListAccountsHandler::timeout()
{
    disconnect(this);
    deleteLater();

    QString err = QString("cannot contact Online Accounts: request timed out after ") +
                  QString::number(timer_.interval()) + " ms";
    qCritical().noquote() << err;
    if (!conn_.send(msg_.createErrorReply(QDBusError::Other, err)))
    {
        auto msg = last_error_msg(conn_);
        qCritical().noquote() << "ListAccounts(): could not send DBus error reply" + msg;
    }
}

} // namespace internal
} // namespace registry
} // namespace storage
} // namespace unity
