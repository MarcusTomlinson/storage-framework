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

class DownloaderImpl;

}  // namespace internal

class Q_DECL_EXPORT Downloader final : public QLocalSocket
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::Downloader::Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::StorageError error READ error NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::Item item READ item NOTIFY statusChanged FINAL)

public:
    enum Status { Loading, Ready, Cancelled, Finished, Error };
    Q_ENUMS(Status)

    Downloader();
    virtual ~Downloader();

    bool isValid() const;
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

private:
    Downloader(std::unique_ptr<internal::DownloaderImpl> p);

    std::unique_ptr<internal::DownloaderImpl> p_;

    friend class internal::DownloaderImpl;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
