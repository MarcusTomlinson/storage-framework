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

#include <QObject>

namespace unity
{
namespace storage
{
namespace qt
{

class Item;
class StorageError;

class Q_DECL_EXPORT ItemListJob final : public QObject
{
    Q_PROPERTY(bool READ isValid FINAL)
    Q_PROPERTY(unity::Storage::ItemJob::Status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::Storage::StorageError READ error FINAL)

public:
    ItemListJob(QObject* parent = nullptr);
    virtual ~ItemListJob();

    enum Status { Loading, Finished, Error };
    Q_ENUM(Status)

    bool isValid() const;
    Status status() const;
    StorageError error() const;

Q_SIGNALS:
    void statusChanged(unity::storage::qt::ItemListJob::Status status) const;
    void itemsReady(QList<unity::storage::qt::Item> const& items) const;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
