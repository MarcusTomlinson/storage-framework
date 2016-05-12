/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
 */

#include <unity/storage/provider/internal/CredentialsCache.h>
#include "businterface.h"

#include <QDBusPendingCallWatcher>

#include <assert.h>
#include <vector>
#include <sys/apparmor.h>

using namespace std;

namespace {

char const DBUS_BUS_NAME[] = "org.freedesktop.DBus";
char const DBUS_BUS_PATH[] = "/org/freedesktop/DBus";

char const UNIX_USER_ID[] = "UnixUserID";
char const PROCESS_ID[] = "ProcessID";
char const LINUX_SECURITY_LABEL[] = "LinuxSecurityLabel";

int const MAX_CACHE_SIZE = 50;

}

namespace unity
{

namespace storage
{

namespace provider
{

namespace internal
{

struct CredentialsCache::Request
{
    QDBusPendingCallWatcher watcher;
    std::vector<boost::promise<CredentialsCache::Credentials>> promises;

    Request(QDBusPendingReply<QVariantMap> const& call) : watcher(call) {}
};

CredentialsCache::CredentialsCache(QDBusConnection const& bus)
    : bus_daemon_(new BusInterface(DBUS_BUS_NAME, DBUS_BUS_PATH, bus))
    , apparmor_enabled_(aa_is_enabled())
{
}

CredentialsCache::~CredentialsCache() = default;

boost::future<CredentialsCache::Credentials> CredentialsCache::get(QString const& peer)
{
    // Return the credentials directly if they are cached
    try
    {
        Credentials const& credentials = cache_.at(peer);
        return boost::make_ready_future<Credentials>(credentials);
    }
    catch (std::out_of_range const &)
    {
        // ignore
    }

    // If the credentials exist in the previous generation of the
    // cache, move them to the current generation.
    try
    {
        Credentials& credentials = old_cache_.at(peer);
        // No real way to get coverage here because we'd
        // need more than 50 peers with different credentials.
        // LCOV_EXCL_START
        cache_.emplace(peer, std::move(credentials));
        old_cache_.erase(peer);
        return boost::make_ready_future<Credentials>(credentials);
        // LCOV_EXCL_STOP
    }
    catch (std::out_of_range const &)
    {
        // ignore
    }

    boost::promise<Credentials> promise;
    auto future = promise.get_future();
    // If the credentials are already being requested, add ourselves
    // to the callback list.
    try
    {
        unique_ptr<Request>& request = pending_.at(peer);
        request->promises.emplace_back(std::move(promise));
        return future;
    }
    catch (std::out_of_range const &)
    {
        // ignore
    }

    // Ask the bus daemon for the peer's credentials
    unique_ptr<Request> request(
        new Request(bus_daemon_->GetConnectionCredentials(peer)));
    QObject::connect(&request->watcher, &QDBusPendingCallWatcher::finished,
                     [this, peer](QDBusPendingCallWatcher *watcher)
                     {
                         this->received_credentials(peer, *watcher);
                     });
    request->promises.emplace_back(std::move(promise));
    pending_.emplace(peer, std::move(request));
    return future;
}

void CredentialsCache::received_credentials(QString const& peer, QDBusPendingReply<QVariantMap> const& reply)
{
    Credentials credentials;
    if (reply.isError())
    {
        // LCOV_EXCL_START
        qWarning() << "CredentialsCache::received_credentials(): "
            "error retrieving credentials for" << peer <<
            ":" << reply.error().message();
        // LCOV_EXCL_STOP
    }
    else
    {
        credentials.valid = true;
        // The contents of this map are described in the specification here:
        // http://dbus.freedesktop.org/doc/dbus-specification.html#bus-messages-get-connection-credentials
        credentials.user = reply.value().value(UNIX_USER_ID).value<uint32_t>();
        credentials.process = reply.value().value(PROCESS_ID).value<uint32_t>();
        if (apparmor_enabled_)
        {
            QByteArray label = reply.value().value(LINUX_SECURITY_LABEL).value<QByteArray>();
            if (label.size() > 0) {
                // The label is null terminated.
                assert(label[label.size()-1] == '\0');
                label.truncate(label.size() - 1);
                // Trim the mode off the end of the label.
                int pos = label.lastIndexOf(' ');
                if (pos > 0 && label.endsWith(')') && label[pos+1] == '(')
                {
                    label.truncate(pos);  // LCOV_EXCL_LINE
                }
                credentials.label = string(label.constData(), label.size());
            }
        }
        else
        {
            // If AppArmor is not enabled, treat peer as unconfined.
            credentials.label = "unconfined";  // LCOV_EXCL_LINE
        }
    }

    // If we've hit our maximum cache size, start a new generation.
    if (cache_.size() >= MAX_CACHE_SIZE)
    {
        // LCOV_EXCL_START
        old_cache_ = std::move(cache_);
        cache_.clear();
        // LCOV_EXCL_STOP
    }
    cache_.emplace(peer, credentials);

    // Notify anyone waiting on the request and remove it from the map:
    for (auto& promise : pending_.at(peer)->promises)
    {
        promise.set_value(credentials);
    }
    pending_.erase(peer);
}

}  // namespace internal

}  // namespace provider

}  // namespace storage

}  // namespace unity
