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

#include <unity/storage/internal/ActivityNotifier.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <OnlineAccounts/Manager>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>
#pragma GCC diagnostic pop

namespace unity
{
namespace storage
{
namespace registry
{
namespace internal
{

class ListAccountsHandler : public QObject
{
    Q_OBJECT

public:
    ListAccountsHandler(QDBusConnection const& conn,
                        QDBusMessage const& msg,
                        std::shared_ptr<storage::internal::InactivityTimer> const& timer);
    ~ListAccountsHandler();

private Q_SLOTS:
    void manager_ready();
    void timeout();

private:
    void initialize_manager();

    QDBusConnection const conn_;
    QDBusMessage const msg_;
    OnlineAccounts::Manager manager_;
    QTimer timer_;
    storage::internal::ActivityNotifier activity_notifier_;  // RAII guard variable

    Q_DISABLE_COPY(ListAccountsHandler)
};

}  // namespace internal
}  // namespace registry
}  // namespace storage
}  // namespace unity
