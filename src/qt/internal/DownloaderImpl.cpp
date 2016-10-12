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

#include "ProviderInterface.h"
#include <unity/storage/qt/internal/Handler.h>
#include <unity/storage/qt/internal/ItemImpl.h>
#include <unity/storage/qt/internal/VoidJobImpl.h>
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

DownloaderImpl::DownloaderImpl(shared_ptr<ItemImpl> const& item_impl,
                               QString const& method,
                               QDBusPendingReply<QString, QDBusUnixFileDescriptor>& reply)
    : status_(Downloader::Status::Loading)
    , item_impl_(item_impl)
{
    assert(item_impl);
    assert(!method.isEmpty());

    auto process_reply = [this, method](decltype(reply)& r)
    {
        if (status_ != Downloader::Status::Loading)
        {
            return;  // Don't transition to a final state more than once.
        }

        auto runtime = item_impl_->runtime_impl();
        if (!runtime || !runtime->isValid())
        {
            QString msg = method + ": Runtime was destroyed previously";
            error_ = StorageErrorImpl::runtime_destroyed_error(msg);
            public_instance_->abort();
            public_instance_->setErrorString(msg);
            status_ = Downloader::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

        download_id_ = r.argumentAt<0>();
        fd_ = r.argumentAt<1>();
        if (fd_.fileDescriptor() < 0)
        {
            // LCOV_EXCL_START
            QString msg = method + ": invalid file descriptor returned by provider";
            qCritical().noquote() << msg;
            error_ = StorageErrorImpl::local_comms_error(msg);
            public_instance_->abort();
            public_instance_->setErrorString(msg);
            status_ = Downloader::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
            // LCOV_EXCL_STOP
        }

        public_instance_->setSocketDescriptor(fd_.fileDescriptor(), QLocalSocket::ConnectedState, QIODevice::ReadOnly);
        status_ = Downloader::Status::Ready;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    auto process_error = [this](StorageError const& error)
    {
        // TODO: This does not set the method
        error_ = error;
        status_ = Downloader::Status::Error;
        public_instance_->abort();
        public_instance_->setErrorString(error.errorString());
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<storage::internal::ItemMetadata>(this, reply, process_reply, process_error);
}

DownloaderImpl::DownloaderImpl(StorageError const& e)
    : status_(Downloader::Status::Error)
    , error_(e)
{
}

DownloaderImpl::~DownloaderImpl()
{
    switch (status_)
    {
        case Downloader::Status::Loading:
        case Downloader::Status::Finished:
        case Downloader::Status::Cancelled:
        case Downloader::Status::Error:
            break;
        case Downloader::Status::Ready:
            public_instance_->abort();
            break;
        default:
            abort();  // Impossible.  // LCOV_EXCL_LINE
    }
}

bool DownloaderImpl::isValid() const
{
    return status_ != Downloader::Status::Error && status_ != Downloader::Status::Cancelled;
}

Downloader::Status DownloaderImpl::status() const
{
    return status_;
}

StorageError DownloaderImpl::error() const
{
    return error_;
}

Item DownloaderImpl::item() const
{
    if (status_ == Downloader::Status::Error)
    {
        return Item();
    }
    return Item(item_impl_);
}

void DownloaderImpl::finishDownload()
{
    static QString const method = "Downloader::finishDownload()";

    // If we encountered an error earlier or were cancelled, or if finishDownload() was
    // called already, we ignore the call.
    if (status_ == Downloader::Status::Error || status_ == Downloader::Status::Cancelled || finalizing_)
    {
        return;
    }

    // Complain if we are asked to finalize while in the Loading or Finished state.
    if (status_ != Downloader::Ready)
    {
        QString msg = method + ": cannot finalize while Downloader is not in the Ready state";
        error_ = StorageErrorImpl::logic_error(msg);
        public_instance_->abort();
        public_instance_->setErrorString(msg);
        status_ = Downloader::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
        return;
    }

    auto runtime = item_impl_->runtime_impl();
    if (!runtime || !runtime->isValid())
    {
        QString msg = method + ": Runtime was destroyed previously";
        error_ = StorageErrorImpl::runtime_destroyed_error(msg);
        status_ = Downloader::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
        return;
    }

    finalizing_ = true;
    auto reply = item_impl_->account_impl()->provider()->FinishDownload(download_id_);

    auto process_reply = [this](decltype(reply)&)
    {
        if (status_ == Downloader::Status::Cancelled || status_ == Downloader::Status::Error)
        {
            return;  // Don't transition to a final state more than once.
        }

        auto runtime = item_impl_->runtime_impl();
        if (!runtime || !runtime->isValid())
        {
            QString msg = method + ": Runtime was destroyed previously";
            error_ = StorageErrorImpl::runtime_destroyed_error(msg);
            public_instance_->abort();
            public_instance_->setErrorString(msg);
            status_ = Downloader::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

        status_ = Downloader::Status::Finished;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    auto process_error = [this](StorageError const& error)
    {
        if (status_ != Downloader::Status::Ready)
        {
            return;  // Don't transition to a final state more than once.
        }

        // TODO: this doesn't set the method
        error_ = error;
        public_instance_->abort();
        public_instance_->setErrorString(error.errorString());
        status_ = Downloader::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<void>(this, reply, process_reply, process_error);
}

void DownloaderImpl::cancel()
{
    static QString const method = "Downloader::cancel()";

    // If we are in a final state already, ignore the call.
    if (   status_ == Downloader::Status::Error
        || status_ == Downloader::Status::Finished
        || status_ == Downloader::Status::Cancelled)
    {
        return;
    }
    auto runtime = item_impl_->runtime_impl();
    if (!runtime || !runtime->isValid())
    {
        QString msg = method + ": Runtime was destroyed previously";
        error_ = StorageErrorImpl::runtime_destroyed_error(msg);
        status_ = Downloader::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
        return;
    }

    QString msg = method + ": download was cancelled";
    error_ = StorageErrorImpl::cancelled_error(msg);
    public_instance_->abort();
    public_instance_->setErrorString(msg);
    status_ = Downloader::Status::Cancelled;
    Q_EMIT public_instance_->statusChanged(status_);
}

Downloader* DownloaderImpl::make_job(shared_ptr<ItemImpl> const& item_impl,
                                     QString const& method,
                                     QDBusPendingReply<QString, QDBusUnixFileDescriptor>& reply)
{
    unique_ptr<DownloaderImpl> impl(new DownloaderImpl(item_impl, method, reply));
    auto downloader = new Downloader(move(impl));
    downloader->p_->public_instance_ = downloader;
    return downloader;
}

Downloader* DownloaderImpl::make_job(StorageError const& e)
{
    unique_ptr<DownloaderImpl> impl(new DownloaderImpl(e));
    auto downloader = new Downloader(move(impl));
    downloader->p_->public_instance_ = downloader;
    QMetaObject::invokeMethod(downloader,
                              "statusChanged",
                              Qt::QueuedConnection,
                              Q_ARG(unity::storage::qt::Downloader::Status, downloader->p_->status_));
    return downloader;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
