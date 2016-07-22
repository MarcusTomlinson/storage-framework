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

#include <unity/storage/qt/client/internal/DownloaderBase.h>

#include <QDBusUnixFileDescriptor>

class ProviderInterface;
class QLocalSocket;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Downloader;

namespace internal
{
namespace remote_client
{

class DownloaderImpl : public DownloaderBase
{
public:
    DownloaderImpl(QString const& download_id,
                   QDBusUnixFileDescriptor fd,
                   std::shared_ptr<File> const& file,
                   std::shared_ptr<ProviderInterface> const& provider);
    virtual ~DownloaderImpl();

    virtual std::shared_ptr<File> file() const override;
    virtual std::shared_ptr<QLocalSocket> socket() const override;
    virtual QFuture<void> finish_download() override;
    virtual QFuture<void> cancel() noexcept override;

    static std::shared_ptr<Downloader> make_downloader(QString const& upload_id,
                                                       QDBusUnixFileDescriptor fd,
                                                       std::shared_ptr<File> const& file,
                                                       std::shared_ptr<ProviderInterface> const& provider);

private:
    QString download_id_;
    QDBusUnixFileDescriptor fd_;
    std::shared_ptr<File> file_;
    std::shared_ptr<ProviderInterface> provider_;
    std::shared_ptr<QLocalSocket> read_socket_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
