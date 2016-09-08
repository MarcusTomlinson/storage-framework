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

#pragma once

#include <unity/storage/qt/internal/dbusmarshal.h>
#include <unity/storage/qt/internal/HandlerBase.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>

#include <QDBusPendingReply>

#include <cassert>
#include <functional>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

template<typename T>
class Handler : public HandlerBase
{
public:
    // TODO: HACK: The reply argument really should be passed by const reference, which also
    //             would make the explicit conversion of the call to QDBusPendingReply<QDBusArgs...>
    //             unnecessary. But this doesn't work with gcc 4.9: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60420
    //             Once we get rid of Vivid, this should be changed back to
    //
    //             Handler<T>::Handler(QObject* parent,
    //                                 QDBusPendingReply<DBusArgs...> const& reply,
    //                                 std::function<void(decltype(reply) const&)> success_closure,
    //                                 std::function<void(StorageError const&) error_closure)

    template<typename ... DBusArgs>
    Handler(QObject* parent,
            QDBusPendingReply<DBusArgs...>& reply,
            std::function<void(decltype(reply)&)> const& success_closure,
            std::function<void(StorageError const&)> const& error_closure)
        : HandlerBase(parent,
                      reply,
                      [this, success_closure, error_closure](QDBusPendingCallWatcher& call)
                          {
                              if (call.isError())
                              {
                                  auto e = unmarshal_exception(call);
                                  switch (e.type())
                                  {
                                      case StorageError::NoError:
                                      {
                                          QString msg = "impossible provider exception: " + e.errorString();
                                          qCritical() << msg;
                                          e = StorageErrorImpl::local_comms_error(msg);
                                          break;
                                      }
                                      case StorageError::LocalCommsError:
                                      case StorageError::RemoteCommsError:
                                      case StorageError::ResourceError:
                                      {
                                          // Log these errors because they are unexpected.
                                          qCritical() << "provider exception:" << e.errorString();
                                          break;
                                      }
                                      // FALLTHROUGH
                                      default:
                                      {
                                          // All other errors are not logged.
                                      }
                                  }
                                  error_closure(e);
                                  return;
                              }
                              // TODO: See HACK above. Should just be success_closure(call);
                              QDBusPendingReply<DBusArgs...> r = call;
                              success_closure(call);
                          })
    {
    }
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
