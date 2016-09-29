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

#include <unity/storage/qt/internal/HandlerBase.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>
#include <unity/storage/qt/internal/unmarshal_error.h>

#include <QDBusPendingReply>
#include <QDebug>

#include <cassert>

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
    template<typename ... DBusArgs>
    Handler(QObject* parent,
            QDBusPendingReply<DBusArgs...> const& reply,
            std::function<void(decltype(reply)&)> const& success_closure,
            std::function<void(StorageError const&)> const& error_closure)
        : HandlerBase(parent,
                      reply,
                      [this, success_closure, error_closure](QDBusPendingCallWatcher& call)
                          {
                              if (call.isError())
                              {
                                  auto e = unmarshal_error(call);
                                  switch (e.type())
                                  {
                                      case StorageError::Type::NoError:
                                      {
                                          // LCOV_EXCL_START
                                          QString msg = "impossible provider exception: " + e.errorString();
                                          qCritical().noquote() << msg;
                                          e = StorageErrorImpl::local_comms_error(msg);
                                          break;
                                          // LCOV_EXCL_STOP
                                      }
                                      case StorageError::Type::LocalCommsError:
                                      case StorageError::Type::RemoteCommsError:
                                      case StorageError::Type::ResourceError:
                                      {
                                          // Log these errors because they are unexpected.
                                          QString msg = "provider exception: " + e.errorString();
                                          qCritical().noquote() << msg;
                                          break;
                                      }
                                      default:
                                      {
                                          // All other errors are not logged.
                                          break;
                                      }
                                  }
                                  error_closure(e);
                                  return;
                              }
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
