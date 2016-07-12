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
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#include <unity/storage/provider/internal/AccountData.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/DBusPeerCache.h>
#include <unity/storage/provider/internal/PendingJobs.h>

#include <OnlineAccounts/AuthenticationData>
#include <QDebug>

using namespace std;

namespace unity {
namespace storage {
namespace provider {
namespace internal {

AccountData::AccountData(std::unique_ptr<ProviderBase>&& provider,
                         std::shared_ptr<DBusPeerCache> const& dbus_peer,
                         QDBusConnection const& bus,
                         OnlineAccounts::Account* account,
                         QObject* parent)
    : QObject(parent), provider_(std::move(provider)), dbus_peer_(dbus_peer),
      jobs_(new PendingJobs(bus)), account_(account)
{
    authenticate(false);
}

AccountData::~AccountData() = default;

ProviderBase& AccountData::provider()
{
    return *provider_;
}

DBusPeerCache& AccountData::dbus_peer()
{
    return *dbus_peer_;
}

PendingJobs& AccountData::jobs()
{
    return *jobs_;
}

void AccountData::authenticate(bool interactive)
{
    // If there is an existing authenticating session running, use
    // that existing session (unless it is a non-interactive session
    // and we've requested interactivity).
    if (auth_watcher_ && (authenticating_interactively_ || !interactive))
    {
        return;
    }

    authenticating_interactively_ = interactive;
    credentials_valid_ = false;
    credentials_ = boost::blank();

    OnlineAccounts::AuthenticationData auth_data(
        account_->authenticationMethod());
    auth_data.setInteractive(interactive);
    OnlineAccounts::PendingCall call = account_->authenticate(auth_data);
    auth_watcher_.reset(new OnlineAccounts::PendingCallWatcher(call));
    connect(auth_watcher_.get(), &OnlineAccounts::PendingCallWatcher::finished,
            this, &AccountData::on_authenticated);
}

bool AccountData::has_credentials()
{
    return credentials_valid_;
}

Credentials const& AccountData::credentials()
{
    return credentials_;
}

void AccountData::on_authenticated()
{
    credentials_ = boost::blank();
    credentials_valid_ = true;
    switch (account_->authenticationMethod()) {
    case OnlineAccounts::AuthenticationMethodOAuth1:
    {
        OnlineAccounts::OAuth1Reply reply(*auth_watcher_);
        if (reply.hasError())
        {
            qDebug() << "Failed to authenticate:" << reply.error().text();
        }
        else
        {
            credentials_ = OAuth1Credentials{
                reply.consumerKey().toStdString(),
                reply.consumerSecret().toStdString(),
                reply.token().toStdString(),
                reply.tokenSecret().toStdString(),
            };
        }
    }
    case OnlineAccounts::AuthenticationMethodOAuth2:
    {
        OnlineAccounts::OAuth2Reply reply(*auth_watcher_);
        if (reply.hasError())
        {
            qDebug() << "Failed to authenticate:" << reply.error().text();
        }
        else
        {
            credentials_ = OAuth2Credentials{
                reply.accessToken().toStdString(),
            };
        }
    }
    case OnlineAccounts::AuthenticationMethodPassword:
    {
        OnlineAccounts::PasswordReply reply(*auth_watcher_);
        if (reply.hasError())
        {
            qDebug() << "Failed to authenticate:" << reply.error().text();
        }
        else
        {
            credentials_ = PasswordCredentials{
                reply.username().toStdString(),
                reply.password().toStdString(),
            };
        }
    }
    default:
        qDebug() << "Unhandled authentication method:"
                 << account_->authenticationMethod();
    }
    auth_watcher_.reset();

    Q_EMIT authenticated();
}

}
}
}
}
