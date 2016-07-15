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

#include <unity/storage/qt/client/internal/ItemBase.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Uploader;

namespace internal
{

class FolderBase : public virtual ItemBase
{
public:
    FolderBase(QString const& identity, ItemType type);

    virtual QFuture<QVector<std::shared_ptr<Item>>> list() const = 0;
    virtual QFuture<QVector<std::shared_ptr<Item>>> lookup(QString const& name) const = 0;
    virtual QFuture<std::shared_ptr<Folder>> create_folder(QString const& name) = 0;
    virtual QFuture<std::shared_ptr<Uploader>> create_file(QString const& name) = 0;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
