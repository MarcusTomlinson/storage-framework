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

#include <unity/storage/qt/Item.h>

#include <QObject>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class ItemJobImpl;

}  // namespace internal

class Item;
class StorageError;

class Q_DECL_EXPORT ItemJob final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::ItemJob::Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::StorageError error READ error NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::Item item READ item NOTIFY statusChanged FINAL)

public:
    virtual ~ItemJob();

    enum Status { Loading, Finished, Error };
    Q_ENUMS(Status)

    bool isValid() const;
    Status status() const;
    StorageError error() const;
    Item item() const;

Q_SIGNALS:
    void statusChanged(unity::storage::qt::ItemJob::Status status) const;

private:
    ItemJob(std::unique_ptr<internal::ItemJobImpl> p);

    std::unique_ptr<internal::ItemJobImpl> const p_;

    friend class internal::ItemJobImpl;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::ItemJob::Status)
