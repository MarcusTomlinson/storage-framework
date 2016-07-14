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

#include <unity/storage/qt/client/Downloader.h>

#include <unity/storage/qt/client/internal/DownloaderBase.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

Downloader::Downloader(internal::DownloaderBase* p)
    : p_(p)
{
}

Downloader::~Downloader() = default;

std::shared_ptr<File> Downloader::file() const
{
    return p_->file();
}

std::shared_ptr<QLocalSocket> Downloader::socket() const
{
    return p_->socket();
}

QFuture<void> Downloader::finish_download()
{
    return p_->finish_download();
}

QFuture<void> Downloader::cancel()
{
    return p_->cancel();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
