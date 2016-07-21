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

#include <unity/storage/qt/client/internal/RuntimeBase.h>

#include <OnlineAccounts/Manager>
#include <QDBusConnection>
#include <QFuture>
#include <QTimer>
#include <QVector>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Account;

namespace internal
{
namespace remote_client
{

class RuntimeImpl : public RuntimeBase
{
    Q_OBJECT

public:
    RuntimeImpl(QDBusConnection const& bus);
    virtual ~RuntimeImpl();

    virtual void shutdown() override;
    virtual QFuture<QVector<std::shared_ptr<Account>>> accounts() override;

    QDBusConnection& connection();

private Q_SLOTS:
    virtual void manager_ready();
    virtual void timeout();

private:
    QDBusConnection conn_;
    std::unique_ptr<OnlineAccounts::Manager> manager_;  // TODO: Hack until we can use the registry
    QTimer timer_;
    QFutureInterface<QVector<std::shared_ptr<Account>>> qf_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
