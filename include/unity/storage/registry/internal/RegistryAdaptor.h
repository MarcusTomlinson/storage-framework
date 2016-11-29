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

#include <unity/storage/internal/AccountDetails.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDBusConnection>
#include <QDBusContext>
#pragma GCC diagnostic pop

#include <memory>

namespace unity
{
namespace storage
{
namespace internal
{

class InactivityTimer;

}  // namespace internal

namespace registry
{
namespace internal
{

class RegistryAdaptor : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    RegistryAdaptor(QDBusConnection const& conn,
                    std::shared_ptr<storage::internal::InactivityTimer> const& timer,
                    QObject* parent = nullptr);
    ~RegistryAdaptor();

public Q_SLOTS:
    QList<unity::storage::internal::AccountDetails> ListAccounts();

private:
    QDBusConnection conn_;
    std::shared_ptr<storage::internal::InactivityTimer> timer_;

    Q_DISABLE_COPY(RegistryAdaptor)
};

}  // namespace internal
}  // namespace registry
}  // namespace storage
}  // namespace unity
