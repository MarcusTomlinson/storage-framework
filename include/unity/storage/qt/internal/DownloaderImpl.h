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

#include <unity/storage/qt/Downloader.h>

#include <QDBusPendingReply>
#include <QDBusUnixFileDescriptor>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class DownloaderImpl : public QObject
{
    Q_OBJECT
public:
    DownloaderImpl(std::shared_ptr<ItemImpl> const& item,
                   QString const& method,
                   QDBusPendingReply<QString, QDBusUnixFileDescriptor> const& reply);
    DownloaderImpl(StorageError const& e);
    ~DownloaderImpl() = default;

    bool isValid() const;
    Downloader::Status status() const;
    StorageError error() const;
    Item item() const;

    void finishDownload();
    void cancel();

    qint64 readData(char* data, qint64 maxSize);
    qint64 writeData(char const* data, qint64 maxSize);

    static Downloader* make_job(std::shared_ptr<ItemImpl> const& item,
                                QString const& method,
                                QDBusPendingReply<QString, QDBusUnixFileDescriptor> const& reply);
    static Downloader* make_job(StorageError const& e);

private:
    Downloader* public_instance_;
    Downloader::Status status_;
    StorageError error_;
    QString method_;
    std::shared_ptr<ItemImpl> item_impl_;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
