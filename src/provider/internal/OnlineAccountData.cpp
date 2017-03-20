/*
 * Copyright (C) 2017 Canonical Ltd
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

#include <unity/storage/provider/internal/OnlineAccountData.h>

#include <OnlineAccounts/AuthenticationData>
#include <QDebug>

using namespace std;
using unity::storage::internal::InactivityTimer;

namespace unity {
namespace storage {
namespace provider {
namespace internal {

OnlineAccountData::OnlineAccountData(shared_ptr<ProviderBase> const& provider,
                                     shared_ptr<DBusPeerCache> const& dbus_peer,
                                     shared_ptr<InactivityTimer> const& inactivity_timer,
                                     QDBusConnection const& bus,
                                     OnlineAccounts::Account* account,
                                     QObject* parent)
    : AccountData(provider, dbus_peer, inactivity_timer, bus, parent),
      account_(account)
{
    connect(account_, &OnlineAccounts::Account::changed,
            this, &OnlineAccountData::on_changed);
    authenticate(false);
}

OnlineAccountData::~OnlineAccountData() = default;

void OnlineAccountData::authenticate(bool interactive, bool invalidate_cache)
{
    // If there is an existing authentication session running, check
    // if it matches our requirements.
    if (auth_watcher_)
    {
        if (invalidate_cache)
        {
            // If invalidate_cache has been requested, the existing
            // session must also be invalidating the cache.
            if (authenticating_invalidate_cache_)
            {
                return;
            }
        }
        else if (interactive)
        {
            // If interactive has been requested, the existing session
            // must also be interactive.
            if (authenticating_interactively_)
            {
                return;
            }
        }
        else
        {
            // Otherwise, any session will do.
            return;
        }
    }

    authenticating_interactively_ = interactive;
    authenticating_invalidate_cache_ = invalidate_cache;
    credentials_ = boost::blank();

    OnlineAccounts::AuthenticationData auth_data(
        account_->authenticationMethod());
    auth_data.setInteractive(interactive);
    if (invalidate_cache)
    {
        auth_data.invalidateCachedReply();
    }
    OnlineAccounts::PendingCall call = account_->authenticate(auth_data);
    auth_watcher_.reset(new OnlineAccounts::PendingCallWatcher(call));
    connect(auth_watcher_.get(), &OnlineAccounts::PendingCallWatcher::finished,
            this, &OnlineAccountData::on_authenticated);
}

bool OnlineAccountData::has_credentials()
{
    // variant index 0 is boost::blank
    return credentials_.which() != 0;
}

Credentials const& OnlineAccountData::credentials()
{
    return credentials_;
}

void OnlineAccountData::on_authenticated()
{
    credentials_ = boost::blank();
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
        break;
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
        break;
    }
    case OnlineAccounts::AuthenticationMethodPassword:
    {
        // Grab hostname from account settings if available
        string host = account_->setting("host").toString().toStdString();

        OnlineAccounts::PasswordReply reply(*auth_watcher_);
        if (reply.hasError())
        {
            qDebug() << "Failed to authenticate:" << reply.error().text();
        }
        else
        {
            QString username = reply.username();
            QString password = reply.password();

            // Work around password credentials bug in online-accounts-service
            //   https://bugs.launchpad.net/bugs/1628473
            if (username.isEmpty() && password.isEmpty())
            {
                username = reply.data()["UserName"].toString();
                password = reply.data()["Secret"].toString();
            }
            credentials_ = PasswordCredentials{
                username.toStdString(),
                password.toStdString(),
                move(host),
            };
        }
        break;
    }
    default:
        qDebug() << "Unhandled authentication method:"
                 << account_->authenticationMethod();
    }
    auth_watcher_.reset();

    Q_EMIT authenticated();
}

void OnlineAccountData::on_changed()
{
    // Assume that if we're in the middle of authenticating that we'll
    // receive valid credentials for the changed account.
    if (auth_watcher_)
    {
        return;
    }
    // Otherwise, invalidate the credentials
    credentials_ = boost::blank();
}

}
}
}
}
