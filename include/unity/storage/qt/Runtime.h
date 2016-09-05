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

#include <unity/storage/qt/StorageError.h>

#include <QDBusConnection>

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

class Q_DECL_EXPORT Runtime final : public QObject
{
    Q_PROPERTY(bool READ isValid)
    Q_PROPERTY(unity::storage::StorageError READ error)
    Q_PROPERTY(QDBusConnection READ connection)
public:
    Runtime(QObject* parent = nullptr);
    Runtime(QDBusConnection const& bus, QObject* parent = nullptr);
    virtual ~Runtime();

    bool isValid() const;
    StorageError error() const;
    QDBusConnection connection() const;
    StorageError shutdown();
    Q_INVOKABLE AccountsJob* accounts() const;

private:
    std::shared_ptr<internal::RuntimeImpl> p_;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
