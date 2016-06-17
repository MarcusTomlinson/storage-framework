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

#pragma once

#include <boost/thread/future.hpp>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QString>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <sys/types.h>

class BusInterface;

namespace unity
{

namespace storage
{

namespace provider
{

namespace internal
{


class DBusPeerCache final {
public:
    struct Credentials
    {
        bool valid = false;
        uid_t uid = 0;
        pid_t pid = 0;
        // Not using QString, because this is not necessarily unicode.
        std::string label;
    };

    DBusPeerCache(QDBusConnection const& bus);
    ~DBusPeerCache();

    DBusPeerCache(DBusPeerCache const&) = delete;
    DBusPeerCache& operator=(DBusPeerCache const&) = delete;

    // Retrieve the security credentials for the given D-Bus peer.
    boost::future<Credentials> get(QString const& peer);

private:
    struct Request;

    std::unique_ptr<BusInterface> bus_daemon_;
    bool apparmor_enabled_;

    std::map<QString,Credentials> cache_;
    std::map<QString,Credentials> old_cache_;
    std::map<QString,std::unique_ptr<Request>> pending_;

    void received_credentials(QString const& peer, QDBusPendingReply<QVariantMap> const& reply);
};

}  // namespace internal

}  // namespace provider

}  // namespace storage

}  // namespace unity
