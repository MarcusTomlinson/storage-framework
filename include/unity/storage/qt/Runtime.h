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

#include <unity/storage/qt/Account.h>
#include <unity/storage/qt/StorageError.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDBusConnection>
#pragma GCC diagnostic pop

#include <memory>

class QDBusConnection;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class RuntimeImpl;

}  // namespace internal

class AccountsJob;

class Q_DECL_EXPORT Runtime : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid FINAL)
    Q_PROPERTY(unity::storage::qt::StorageError error READ error FINAL)
    Q_PROPERTY(QDBusConnection connection READ connection CONSTANT FINAL)

public:
    Runtime(QObject* parent = nullptr);
    Runtime(QDBusConnection const& bus, QObject* parent = nullptr);
    virtual ~Runtime();

    bool isValid() const;
    StorageError error() const;
    QDBusConnection connection() const;
    StorageError shutdown();
    Q_INVOKABLE unity::storage::qt::AccountsJob* accounts() const;

    Account make_test_account(QString const& bus_name,
                              QString const& object_path,
                              QString const& id = "",
                              QString const& serviceId = "",
                              QString const& displayName = "") const;

private:
    std::shared_ptr<internal::RuntimeImpl> p_;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
