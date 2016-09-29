/*
 * Copyright 2016 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#include "plugin.h"
#include <unity/storage/qt/Runtime.h>
#include <unity/storage/qt/Account.h>
#include <unity/storage/qt/AccountsJob.h>
#include <unity/storage/qt/Item.h>
#include <unity/storage/qt/ItemJob.h>
#include <unity/storage/qt/ItemListJob.h>

using namespace unity::storage::qt;

namespace unity
{
namespace storage
{
namespace qml
{

void StorageFrameworkPlugin::registerTypes(const char* uri)
{
    qmlRegisterType<Runtime>(uri, 0, 1, "Runtime");
    qmlRegisterUncreatableType<AccountsJob>(uri, 0, 1, "AccountsJob", "Use Runtime to create AccountsJob");
    qmlRegisterUncreatableType<Account>(uri, 0, 1, "Account", "");
    qmlRegisterUncreatableType<Item>(uri, 0, 1, "Item", "");
    qmlRegisterUncreatableType<ItemJob>(uri, 0, 1, "ItemJob", "Use Account or another item to access items");
    qmlRegisterUncreatableType<ItemListJob>(uri, 0, 1, "ItemListJob", "Use Account or another item to access items");
}

}
}
}
