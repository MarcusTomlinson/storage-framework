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

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/HandlerBase.h>

#include <QDBusPendingReply>
#include <QFuture>
#include <QFutureInterface>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{

template<typename T>
class Handler : public HandlerBase
{
public:
    template<typename ... DBusArgs>
    Handler(QObject* parent,
            QDBusPendingReply<DBusArgs...> const& reply,
            std::function<void(decltype(reply) const&, QFutureInterface<T>&)> closure);

    QFuture<T> future();

private:
    QFutureInterface<T> qf_;
};

template<typename T>
template<typename ... DBusArgs>
Handler<T>::Handler(QObject* parent,
                    QDBusPendingReply<DBusArgs...> const& reply,
                    std::function<void(decltype(reply) const&, QFutureInterface<T>&)> closure)
    : HandlerBase(parent,
                  reply,
                  [this, closure](QDBusPendingCallWatcher const& call)
                      {
                          if (call.isError())
                          {
                              qDebug() << call.error().message();  // TODO, remove this
                              qf_.reportException(StorageException());  // TODO
                              qf_.reportFinished();
                              return;
                          }
                          closure(call, qf_);
                      })
{
    qf_.reportStarted();
}

template<typename T>
QFuture<T> Handler<T>::future()
{
    return qf_.future();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
