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

#include <unity/storage/qt/client/internal/remote_client/HandlerBase.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

#include <cassert>

using namespace std;

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

HandlerBase::HandlerBase(QObject* parent,
                         QDBusPendingCall const& call,
                         function<void(QDBusPendingCallWatcher const&)> const& closure)
    : QObject(parent)
    , watcher_(call)
    , closure_(closure)
{
    assert(closure);
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &HandlerBase::finished);
}

void HandlerBase::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();
    closure_(*call);
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
