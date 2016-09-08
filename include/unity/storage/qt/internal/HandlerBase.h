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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <QDBusPendingCallWatcher>
#pragma GCC diagnostic pop
#include <QObject>

#include <functional>

class QDBusPendingCall;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class HandlerBase : public QObject
{
    Q_OBJECT

public:
    // TODO: HACK: gcc 4.9 bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60420
    //             Once we get rid of Vivid, this should be changed back to
    //
    //             HandlerBase(QObject* parent,
    //                         QDBusPendingCall const& call,
    //                         std::function<void(QDBusPendingCallWatcher const&)> const& closure);
    HandlerBase(QObject* parent,
                QDBusPendingCall& call,
                std::function<void(QDBusPendingCallWatcher&)> const& closure);

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

protected:
    QDBusPendingCallWatcher watcher_;

private:
    std::function<void(QDBusPendingCallWatcher&)> closure_;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
