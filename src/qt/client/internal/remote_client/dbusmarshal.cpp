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

#include <unity/storage/qt/client/internal/remote_client/dbusmarshal.h>

#include <unity/storage/internal/dbus_error.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/make_future.h>

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>

#include <cassert>
#include <functional>
#include <map>

using namespace unity::storage::internal;
using namespace std;

namespace unity
{
namespace storage
{
namespace internal
{

QDBusArgument& operator<<(QDBusArgument& argument, storage::internal::ItemMetadata const& metadata)
{
    argument.beginStructure();
    argument << metadata.item_id;
    argument << metadata.parent_id;
    argument << metadata.name;
    argument << metadata.etag;
    argument << static_cast<int32_t>(metadata.type);
    argument.beginMap(QVariant::String, qMetaTypeId<QDBusVariant>());
    decltype(ItemMetadata::metadata)::const_iterator i = metadata.metadata.constBegin();
    while (i != metadata.metadata.constEnd())
    {
        argument.beginMapEntry();
        argument << i.key() << QDBusVariant(i.value());
        argument.endMapEntry();
        ++i;
    }
    argument.endMap();
    argument.endStructure();
    return argument;
}

QDBusArgument const& operator>>(QDBusArgument const& argument, storage::internal::ItemMetadata& metadata)
{
    argument.beginStructure();
    argument >> metadata.item_id;
    argument >> metadata.parent_id;
    argument >> metadata.name;
    argument >> metadata.etag;
    int32_t enum_val;
    argument >> enum_val;
    if (enum_val < 0 || enum_val >= int(ItemType::LAST_ENTRY__))
    {
        qCritical() << "unmarshaling error: impossible ItemType value: " + QString::number(enum_val);
        return argument;  // Forces error
    }
    metadata.type = static_cast<ItemType>(enum_val);
    metadata.metadata.clear();
    argument.beginMap();
    while (!argument.atEnd())
    {
        QString key;
        QVariant value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        metadata.metadata.insert(key, value);
    }
    argument.endMap();
    argument.endStructure();
    return argument;
}

QDBusArgument& operator<<(QDBusArgument& argument, QList<storage::internal::ItemMetadata> const& md_list)
{
    argument.beginArray(qMetaTypeId<storage::internal::ItemMetadata>());
    for (auto const& md : md_list)
    {
        argument << md;
    }
    argument.endArray();
    return argument;
}

QDBusArgument const& operator>>(QDBusArgument const& argument, QList<storage::internal::ItemMetadata>& md_list)
{
    md_list.clear();
    argument.beginArray();
    while (!argument.atEnd())
    {
        ItemMetadata imd;
        argument >> imd;
        md_list.append(imd);
    }
    argument.endArray();
    return argument;
}

}  // namespace internal

namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{
namespace
{

template<typename T>
exception_ptr make_exception(QDBusPendingCallWatcher const& call)
{
    QDBusPendingReply<QString> reply = call;
    auto msg = reply.argumentAt<0>();
    return make_exception_ptr(T(msg));
}

template<>
exception_ptr make_exception<NotExistsException>(QDBusPendingCallWatcher const& call)
{
    QDBusPendingReply<QString, QString> reply = call;
    auto msg = reply.argumentAt<0>();
    auto key = reply.argumentAt<1>();
    return make_exception_ptr(NotExistsException(msg, key));
}

template<>
exception_ptr make_exception<ExistsException>(QDBusPendingCallWatcher const& call)
{
    QDBusPendingReply<QString, QString, QString> reply = call;
    auto msg = reply.argumentAt<0>();
    auto id = reply.argumentAt<1>();
    auto name = reply.argumentAt<2>();
    return make_exception_ptr(ExistsException(msg, id, name));
}

template<>
exception_ptr make_exception<ResourceException>(QDBusPendingCallWatcher const& call)
{
    QDBusPendingReply<QString, int> reply = call;
    auto msg = reply.argumentAt<0>();
    auto error_code = reply.argumentAt<1>();
    return make_exception_ptr(ResourceException(msg, error_code));
}

static const map<QString, function<exception_ptr(QDBusPendingCallWatcher const& call)>> exception_factories =
{
    { "NotExistsException",       make_exception<NotExistsException> },
    { "ExistsException",          make_exception<ExistsException> },
    { "ResourceException",        make_exception<ResourceException> },
    { "RemoteCommsException",     make_exception<RemoteCommsException> },
    { "ConflictException",        make_exception<ConflictException> },
    { "PermissionException",      make_exception<PermissionException> },
    { "QuotaException",           make_exception<QuotaException> },
    { "CancelledException",       make_exception<CancelledException> },
    { "LogicException",           make_exception<LogicException> },
    { "InvalidArgumentException", make_exception<InvalidArgumentException> },
    { "UnknownException",         make_exception<LocalCommsException> }  // Yes, LocalCommsException is intentional
};

}  // namespace

std::exception_ptr unmarshal_exception(QDBusPendingCallWatcher const& call)
{
    assert(call.isError());

    int err = call.error().type();
    if (err != QDBusError::Other)
    {
        return make_exception_ptr(LocalCommsException(call.error().message()));
    }

    auto exception_type = call.error().name();
    if (!exception_type.startsWith(DBUS_ERROR_PREFIX))
    {
        QString msg = "unmarshal_exception(): impossible exception type received from server: " + exception_type;
        return make_exception_ptr(LocalCommsException(msg));
    }
    exception_type = exception_type.remove(0, strlen(DBUS_ERROR_PREFIX));

    auto factory_it = exception_factories.find(exception_type);
    if (factory_it == exception_factories.end())
    {
        QString msg = "unmarshal_exception(): impossible exception type received from server: " + exception_type;
        return make_exception_ptr(LocalCommsException(msg));
    }
    return factory_it->second(call);
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
