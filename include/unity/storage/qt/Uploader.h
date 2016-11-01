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
#include <unity/storage/qt/StorageError.h>

#include <QLocalSocket>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class UploaderImpl;

}  // namespace internal

class Item;
class StorageError;

class Q_DECL_EXPORT Uploader final : public QIODevice
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::Uploader::Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::StorageError error READ error NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::Item::ConflictPolicy policy READ policy NOTIFY statusChanged FINAL)
    Q_PROPERTY(qint64 sizeInBytes READ sizeInBytes NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::Item item READ item NOTIFY statusChanged FINAL)

public:
    enum Status { Loading, Ready, Cancelled, Finished, Error };
    Q_ENUMS(Status)

    Uploader();
    virtual ~Uploader();

    bool isValid() const;
    Status status() const;
    StorageError error() const;
    Item::ConflictPolicy policy() const;
    qint64 sizeInBytes() const;
    Item item() const;

    Q_INVOKABLE void finishUpload();
    Q_INVOKABLE void cancel();

    // From QLocalSocket interface.
    Q_INVOKABLE qint64 bytesAvailable() const override;
    Q_INVOKABLE qint64 bytesToWrite() const override;
    Q_INVOKABLE bool isSequential() const override;
    Q_INVOKABLE bool waitForBytesWritten(int msecs = 30000) override;
    Q_INVOKABLE bool waitForReadyRead(int msecs = 30000) override;

Q_SIGNALS:
    void statusChanged(unity::storage::qt::Uploader::Status status) const;

private:
    Uploader(std::unique_ptr<internal::UploaderImpl> p);

    Q_INVOKABLE qint64 readData(char* data, qint64 c);
    Q_INVOKABLE qint64 writeData(char const* data, qint64 c);

    std::unique_ptr<internal::UploaderImpl> p_;

    friend class internal::UploaderImpl;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::Uploader::Status)
