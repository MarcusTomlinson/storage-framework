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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#include <QObject>
#pragma GCC diagnostic pop

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class ListJobImplBase;
class ItemListJobImpl;
class MultiItemJobImpl;
class MultiItemListJobImpl;

}  // namespace internal

class Item;
class StorageError;

class Q_DECL_EXPORT ItemListJob final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::ItemListJob::Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::StorageError error READ error NOTIFY statusChanged FINAL)

public:
    virtual ~ItemListJob();

    enum Status { Loading, Finished, Error };
    Q_ENUMS(Status)

    bool isValid() const;
    Status status() const;
    StorageError error() const;

Q_SIGNALS:
    void statusChanged(unity::storage::qt::ItemListJob::Status status) const;
    void itemsReady(QList<unity::storage::qt::Item> const& items) const;

private:
    ItemListJob(std::unique_ptr<internal::ListJobImplBase> p);

    std::unique_ptr<internal::ListJobImplBase> const p_;

    friend class internal::ListJobImplBase;
    friend class internal::ItemListJobImpl;
    friend class internal::MultiItemJobImpl;
    friend class internal::MultiItemListJobImpl;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::ItemListJob::Status)
