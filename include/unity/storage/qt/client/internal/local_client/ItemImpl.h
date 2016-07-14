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

#include <unity/storage/common.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/boost_filesystem.h>
#include <unity/storage/qt/client/internal/ItemBase.h>

#include <mutex>

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

class ItemImpl : public virtual ItemBase
{
public:
    ItemImpl(QString const& identity, ItemType type);
    virtual ~ItemImpl();

    virtual QString name() const override;
    virtual QString etag() const override;
    virtual QVariantMap metadata() const override;
    virtual QDateTime last_modified_time() const override;
    virtual QFuture<std::shared_ptr<Item>> copy(std::shared_ptr<Folder> const& new_parent, QString const& new_name) override;
    virtual QFuture<std::shared_ptr<Item>> move(std::shared_ptr<Folder> const& new_parent, QString const& new_name) override;
    virtual QFuture<QVector<std::shared_ptr<Folder>>> parents() const override;
    virtual QVector<QString> parent_ids() const override;
    virtual QFuture<void> delete_item() override;
    virtual bool equal_to(ItemBase const& other) const noexcept override;

    QDateTime get_modified_time();
    void update_modified_time();

    std::unique_lock<std::mutex> get_lock();

protected:
    static boost::filesystem::path sanitize(QString const& name, QString const& method);
    static bool is_reserved_path(boost::filesystem::path const& path) noexcept;

    DeletedException deleted_ex(QString const& method) const noexcept;

    bool deleted_;
    QString name_;
    QString etag_;
    QDateTime modified_time_;
    QVariantMap metadata_;
    std::mutex mutable mutex_;
};

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
