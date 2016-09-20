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

#include <QIODevice>

namespace unity
{
namespace storage
{
namespace qt
{

class Item;
class StorageError;

class Q_DECL_EXPORT Downloader final : public QIODevice
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid FINAL) // TODO: Need notify and constant where appropriate
    Q_PROPERTY(unity::Storage::qt::Downloader::Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::Storage::qt::StorageError error READ error FINAL)
    Q_PROPERTY(unity::Storage::qt::Item item READ item FINAL)

public:
    enum Status { Loading, Ready, Cancelled, Finished, Error };
    Q_ENUM(Status)

    Downloader();
    virtual ~Downloader();

    bool isValid();
    Status status() const;
    StorageError error() const;
    Item item() const;

    Q_INVOKABLE void finishDownload(); // TODO: finish()
    Q_INVOKABLE void cancel();

    // TODO: will probably need QML invokable methods for reading and writing to/from QIODevice

Q_SIGNALS:
    void statusChanged(unity::storage::qt::Downloader::Status status) const;

protected:
    virtual qint64 readData(char* data, qint64 maxSize) override;
    virtual qint64 writeData(char const* data, qint64 maxSize) override;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
