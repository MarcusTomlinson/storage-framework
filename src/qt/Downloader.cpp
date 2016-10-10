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

#include <unity/storage/qt/Downloader.h>

#include <unity/storage/qt/internal/DownloaderImpl.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{

Downloader::Downloader() = default;

Downloader::Downloader(unique_ptr<internal::DownloaderImpl> p)
    : p_(move(p))
{
    assert(p);
}

Downloader::~Downloader() = default;

bool Downloader::isValid() const
{
    return p_->isValid();
}

Downloader::Status Downloader::status() const
{
    return p_->status();
}

StorageError Downloader::error() const
{
    return p_->error();
}

Item Downloader::item() const
{
    return p_->item();
}

void Downloader::finishDownload()
{
    p_->finishDownload();
}

void Downloader::cancel()
{
    p_->cancel();
}

qint64 Downloader::readData(char* data, qint64 maxSize)
{
    return p_->readData(data, maxSize);
}

qint64 Downloader::writeData(char const* data, qint64 maxSize)
{
    return p_->writeData(data, maxSize);
}

}  // namespace qt
}  // namespace storage
}  // namespace unity
