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

#include <unity/storage/qt/client/internal/RootBase.h>
#include <unity/storage/qt/client/internal/local_client/FolderImpl.h>

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
namespace local_client
{

class RootImpl : public virtual RootBase, public virtual FolderImpl
{
public:
    RootImpl(QString const& identity, std::weak_ptr<Account> const& account);

    virtual QString name() const override;
    virtual QFuture<QVector<std::shared_ptr<Folder>>> parents() const override;
    virtual QVector<QString> parent_ids() const override;
    virtual QFuture<void> delete_item() override;

    virtual QFuture<int64_t> free_space_bytes() const override;
    virtual QFuture<int64_t> used_space_bytes() const override;
    virtual QFuture<Item::SPtr> get(QString native_identity) const override;

    static std::shared_ptr<Root> make_root(QString const& identity, std::weak_ptr<Account> const& account);
};

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
