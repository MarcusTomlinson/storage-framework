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

class Q_DECL_EXPORT ItemJob final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool READ isValid)
    Q_PROPERTY(unity::Storage::ItemJob::Status READ status NOTIFY statusChanged)
    Q_PROPERTY(unity::Storage::StorageError READ error)
    Q_PROPERTY(unity::Storage::Item const& READ item)

public:
    ItemJob(QObject* parent = nullptr);
    virtual ~ItemJob();

    enum Status { Loading, Finished, Error };
    Q_ENUM(Status)

    bool isValid() const;
    Status status() const;
    StorageError error() const;
    Item item() const;

Q_SIGNALS:
    void statusChanged(unity::Storage::ItemJob::Status status) const;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
