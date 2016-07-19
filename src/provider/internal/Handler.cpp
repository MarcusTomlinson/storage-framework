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

#include <unity/storage/provider/internal/Handler.h>
#include <unity/storage/provider/internal/AccountData.h>
#include <unity/storage/provider/internal/DBusPeerCache.h>
#include <unity/storage/provider/internal/MainLoopExecutor.h>
#include <unity/storage/provider/ProviderBase.h>

#include <stdexcept>

using namespace std;

namespace
{
char const ERROR[] = "com.canonical.StorageFramework.Provider.Error";
}

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

Handler::Handler(shared_ptr<AccountData> const& account,
                 Callback const& callback,
                 QDBusConnection const& bus, QDBusMessage const& message)
    : account_(account), callback_(callback), bus_(bus), message_(message)
{
}

void Handler::begin()
{
    // Need to put security check in here.
    auto peer_future = account_->dbus_peer().get(message_.service());
    creds_future_ = peer_future.then(
        EXEC_IN_MAIN
        [this](decltype(peer_future) f) {
            auto info = f.get();
            if (info.valid)
            {
                context_ = {info.uid, info.pid, std::move(info.label),
                            account_->credentials()};
                QMetaObject::invokeMethod(this, "credentials_received");
            }
            else
            {
                reply_ = message_.createErrorReply(
                    ERROR, "Handler::begin(): could not retrieve credentials");
                QMetaObject::invokeMethod(this, "send_reply");
            }
        });
}

void Handler::credentials_received()
{
    auto msg_future = callback_(account_, context_, message_);
    reply_future_ = msg_future.then(
        EXEC_IN_MAIN
        [this](decltype(msg_future) f) {
            try
            {
                reply_ = f.get();
            }
            catch (std::exception const& e)
            {
                reply_ = message_.createErrorReply(ERROR, e.what());
            }
            QMetaObject::invokeMethod(this, "send_reply");
        });
}

void Handler::send_reply()
{
    bus_.send(reply_);
    Q_EMIT finished();
}

}
}
}
}
