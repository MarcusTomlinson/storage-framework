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

#include <unity/storage/qt/internal/unmarshal_error.h>

#include <unity/storage/internal/dbus_error.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>

#include <QDBusPendingReply>

#include <cassert>
#include <map>

using namespace unity::storage::internal;
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

template<StorageError::Type T>
StorageError make_error(QDBusPendingCallWatcher const& call)
{
    QDBusPendingReply<QString> reply = call;
    auto msg = reply.argumentAt<0>();
    return StorageErrorImpl::make_error(T, msg);
}

template<>
StorageError make_error<StorageError::Type::NotExists>(QDBusPendingCallWatcher const& call)
{
    QDBusPendingReply<QString, QString> reply = call;
    auto msg = reply.argumentAt<0>();
    auto key = reply.argumentAt<1>();
    return StorageErrorImpl::not_exists_error(msg, key);
}

template<>
StorageError make_error<StorageError::Type::Exists>(QDBusPendingCallWatcher const& call)
{
    QDBusPendingReply<QString, QString, QString> reply = call;
    auto msg = reply.argumentAt<0>();
    auto id = reply.argumentAt<1>();
    auto name = reply.argumentAt<2>();
    return StorageErrorImpl::exists_error(msg, id, name);
}

template<>
StorageError make_error<StorageError::Type::ResourceError>(QDBusPendingCallWatcher const& call)
{
    QDBusPendingReply<QString, int> reply = call;
    auto msg = reply.argumentAt<0>();
    auto error_code = reply.argumentAt<1>();
    return StorageErrorImpl::resource_error(msg, error_code);
}

static const map<QString, function<StorageError(QDBusPendingCallWatcher const& call)>> exception_factories =
{
    { "RemoteCommsException",     make_error<StorageError::Type::RemoteCommsError> },
    { "NotExistsException",       make_error<StorageError::Type::NotExists> },
    { "ExistsException",          make_error<StorageError::Type::Exists> },
    { "ConflictException",        make_error<StorageError::Type::Conflict> },
    { "UnauthorizedException",    make_error<StorageError::Type::Unauthorized> },
    { "PermissionException",      make_error<StorageError::Type::PermissionDenied> },
    { "CancelledException",       make_error<StorageError::Type::Cancelled> },
    { "LogicException",           make_error<StorageError::Type::LogicError> },
    { "InvalidArgumentException", make_error<StorageError::Type::InvalidArgument> },
    { "ResourceException",        make_error<StorageError::Type::ResourceError> },
    { "QuotaException",           make_error<StorageError::Type::QuotaExceeded> },
    { "UnknownException",         make_error<StorageError::Type::LocalCommsError> }  // Yes, LocalCommsError is intentional
};

}  // namespace

StorageError unmarshal_error(QDBusPendingCallWatcher const& call)
{
    assert(call.isError());

    int err = call.error().type();
    if (err != QDBusError::Other)
    {
        // Some DBus error that doesn't represent a StorageError.
        return StorageErrorImpl::local_comms_error(call.error().message());
    }

    auto exception_type = call.error().name();
    if (!exception_type.startsWith(DBUS_ERROR_PREFIX))
    {
        // Some error with the wrong prefix (should never happen unless the server is broken).
        QString msg = "unmarshal_exception(): unknown exception type received from server: " + exception_type
                      + ": " + call.error().message();
        return StorageErrorImpl::local_comms_error(msg);
    }
    exception_type = exception_type.remove(0, strlen(DBUS_ERROR_PREFIX));

    auto factory_it = exception_factories.find(exception_type);
    if (factory_it == exception_factories.end())
    {
        // Some StorageError that we don't recognize.
        QString msg = "unmarshal_exception(): unknown exception type received from server: " + exception_type
                      + ": " + call.error().message();
        return StorageErrorImpl::local_comms_error(msg);
    }
    return factory_it->second(call);
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
