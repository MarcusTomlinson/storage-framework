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

#include <QIODevice>

namespace unity
{
namespace storage
{
namespace qt
{

class Item;
class StorageError;

class Q_DECL_EXPORT Uploader final : public QIODevice
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid NOTIFY statusChanged FINAL) // TODO: Need notify
    Q_PROPERTY(unity::storage::qt::Uploader::Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::StorageError READ error NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::Item::ConflictPolicy policy READ policy CONSTANT FINAL)
    Q_PROPERTY(qint64 sizeInBytes READ sizeInBytes CONSTANT FINAL)
    Q_PROPERTY(unity::storage::qt::Item item READ item NOTIFY statusChanged FINAL)

public:
    enum Status { Loading, Cancelled, Finished, Error };
    Q_ENUMS(Status)

    Uploader();
    virtual ~Uploader();

    bool isValid() const;
    Status status() const;
    StorageError error() const;
    ConflictPolicy policy() const;
    qint64 sizeInBytes() const;
    Item item() const;

    Q_INVOKABLE void finishUpload();
    Q_INVOKABLE void cancel();

Q_SIGNALS:
    void statusChanged(unity::storage::qt::Uploader::Status status) const;

protected:
    virtual qint64 readData(char* data, qint64 maxSize) override;
    virtual qint64 writeData(char const* data, qint64 maxSize) override;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
