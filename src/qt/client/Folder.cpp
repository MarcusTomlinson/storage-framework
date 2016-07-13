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

#include <unity/storage/qt/client/Folder.h>

#include <unity/storage/qt/client/internal/FolderBase.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

using namespace internal;
using namespace std;

Folder::Folder(FolderBase* p)
    : Item(p)
{
}

Folder::~Folder() = default;

QFuture<QVector<Item::SPtr>> Folder::list() const
{
    return dynamic_cast<FolderBase*>(p_.get())->list();
}

QFuture<QVector<Item::SPtr>> Folder::lookup(QString const& name) const
{
    return dynamic_cast<FolderBase*>(p_.get())->lookup(name);
}

QFuture<Folder::SPtr> Folder::create_folder(QString const& name)
{
    return dynamic_cast<FolderBase*>(p_.get())->create_folder(name);
}

QFuture<shared_ptr<Uploader>> Folder::create_file(QString const& name)
{
    return dynamic_cast<FolderBase*>(p_.get())->create_file(name);
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
