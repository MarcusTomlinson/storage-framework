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

#include <unity/storage/registry/internal/RegistryAdaptor.h>

#include <unity/storage/registry/internal/ListAccountsHandler.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace registry
{
namespace internal
{

RegistryAdaptor::RegistryAdaptor(QString const& prog_name,
                                 QDBusConnection const& conn,
                                 shared_ptr<storage::internal::InactivityTimer> const& timer,
                                 QObject* parent)
    : QObject(parent)
    , conn_(conn)
    , prog_name_(prog_name)
    , timer_(timer)
{
}

RegistryAdaptor::~RegistryAdaptor() = default;

QList<unity::storage::internal::AccountDetails> RegistryAdaptor::ListAccounts()
{
    new ListAccountsHandler(prog_name_, conn_, message(), timer_);  // Handler deletes itself once done.
    setDelayedReply(true);
    return QList<unity::storage::internal::AccountDetails>();
}

} // namespace internal
} // namespace registry
} // namespace storage
} // namespace unity
