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

#include <unity/storage/qt/internal/DownloaderImpl.h>

#include <unity/storage/qt/internal/Handler.h>
#include <unity/storage/qt/internal/ItemImpl.h>
#include <unity/storage/qt/ItemJob.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

DownloaderImpl::DownloaderImpl(shared_ptr<ItemImpl> const& item,
                               QString const& method,
                               QDBusPendingReply<QString, QDBusUnixFileDescriptor> const& reply)
    : status_(Downloader::Loading)
    , method_(method)
    , item_impl_(item)
{
    assert(item);
    assert(!method.isEmpty());

    auto process_reply = [this](decltype(reply)& r)
    {
        auto runtime = item_impl_->runtime();
        if (!runtime || !runtime->isValid())
        {
            error_ = StorageErrorImpl::runtime_destroyed_error(method_ + ": Runtime was destroyed previously");
            status_ = Downloader::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

#if 0
        auto metadata = r.value();
        try
        {
            validate_(metadata);
            item_ = ItemImpl::make_item(method_, metadata, account_);
            status_ = ItemJob::Status::Finished;
        }
        catch (StorageError const& e)
        {
            // Bad metadata received from provider, validate_() or make_item() have logged it.
            error_ = e;
            status_ = ItemJob::Status::Error;
        }
#endif
        Q_EMIT public_instance_->statusChanged(status_);
    };

    auto process_error = [this](StorageError const& error)
    {
        // TODO: This does not set the method
        error_ = error;
        status_ = Downloader::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<storage::internal::ItemMetadata>(this, reply, process_reply, process_error);
}

DownloaderImpl::DownloaderImpl(StorageError const& e)
    : status_(Downloader::Error)
    , error_(e)
{
}

bool DownloaderImpl::isValid() const
{
    return false;  // TODO
}

Downloader::Status DownloaderImpl::status() const
{
    return Downloader::Status::Cancelled;  // TODO
}

StorageError DownloaderImpl::error() const
{
    return StorageError();  // TODO
}

Item DownloaderImpl::item() const
{
    return Item();  // TODO
}

void DownloaderImpl::finishDownload()
{
    // TODO
}

void DownloaderImpl::cancel()
{
    // TODO
}

qint64 DownloaderImpl::readData(char* data, qint64 maxSize)
{
    return 0;  // TODO
}

qint64 DownloaderImpl::writeData(char const* data, qint64 maxSize)
{
    // Downloader can only be used to read from.
    return -1;
}

Downloader* DownloaderImpl::make_job(shared_ptr<ItemImpl> const& item,
                                     QString const& method,
                                     QDBusPendingReply<QString, QDBusUnixFileDescriptor> const& reply)
{
    unique_ptr<DownloaderImpl> impl(new DownloaderImpl(item, method, reply));
    auto downloader = new Downloader(move(impl));
    downloader->p_->public_instance_ = downloader;
    return downloader;
}

Downloader* DownloaderImpl::make_job(StorageError const& e)
{
    unique_ptr<DownloaderImpl> impl(new DownloaderImpl(e));
    auto downloader = new Downloader(move(impl));
    downloader->p_->public_instance_ = downloader;
    return downloader;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
