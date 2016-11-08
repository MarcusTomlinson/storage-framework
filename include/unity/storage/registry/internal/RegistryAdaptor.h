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

namespace unity
{
namespace storage
{
namespace registry
{
namespace internal
{

class RegistryAdaptor : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    RegistryAdaptor(QString const& prog_name, QDBusConnection const& conn, QObject* parent = nullptr);
    ~RegistryAdaptor();

public Q_SLOTS:
    QList<unity::storage::internal::AccountDetails> ListAccounts();

private:
    QDBusConnection conn_;
    QString prog_name_;

    Q_DISABLE_COPY(RegistryAdaptor)
};

}  // namespace internal
}  // namespace registry
}  // namespace storage
}  // namespace unity
