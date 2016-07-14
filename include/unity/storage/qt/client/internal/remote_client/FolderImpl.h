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

#include <unity/storage/qt/client/internal/FolderBase.h>
#include <unity/storage/qt/client/internal/remote_client/ItemImpl.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{

class FolderImpl : public virtual FolderBase, public virtual ItemImpl
{
public:
    FolderImpl(storage::internal::ItemMetadata const& md);
    FolderImpl(storage::internal::ItemMetadata const& md, ItemType type);

    QFuture<QVector<std::shared_ptr<Item>>> list() const override;
    QFuture<QVector<std::shared_ptr<Item>>> lookup(QString const& name) const override;
    QFuture<std::shared_ptr<Folder>> create_folder(QString const& name) override;
    QFuture<std::shared_ptr<Uploader>> create_file(QString const& name) override;

    static std::shared_ptr<Folder> make_folder(storage::internal::ItemMetadata const& md, std::weak_ptr<Root> root);
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
