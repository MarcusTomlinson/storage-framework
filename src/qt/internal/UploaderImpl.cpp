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

#include <unity/storage/qt/internal/UploaderImpl.h>

#include "ProviderInterface.h"
//#include <unity/storage/qt/internal/Handler.h>
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

UploaderImpl::UploaderImpl(shared_ptr<ItemImpl> const& item_impl,
                           QString const& method,
                           QDBusPendingReply<QString, QDBusUnixFileDescriptor>& reply,
                           std::function<void(storage::internal::ItemMetadata const&)> const& validate,
                           Item::ConflictPolicy policy,
                           qint64 size_in_bytes)
    : status_(Uploader::Status::Loading)
    , method_(method)
    , item_impl_(item_impl)
    , validate_(validate)
    , policy_(policy)
    , size_in_bytes_(size_in_bytes)
{
    assert(item_impl);
    assert(validate);
    assert(!method.isEmpty());
    assert(size_in_bytes >= 0);

    process_reply_ = [this, method](QDBusPendingReply<QString, QDBusUnixFileDescriptor>& r)
    {
        if (status_ != Uploader::Status::Loading)
        {
            return;  // Don't transition to a final state more than once.
        }

        auto runtime = item_impl_->runtime_impl();
        if (!runtime || !runtime->isValid())
        {
            QString msg = method + ": Runtime was destroyed previously";
            error_ = StorageErrorImpl::runtime_destroyed_error(msg);
            socket_.abort();
            public_instance_->setErrorString(msg);
            status_ = Uploader::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

        upload_id_ = r.argumentAt<0>();
        fd_ = r.argumentAt<1>();
        if (fd_.fileDescriptor() < 0)
        {
            // LCOV_EXCL_START
            QString msg = method + ": invalid file descriptor returned by provider";
            qCritical().noquote() << msg;
            error_ = StorageErrorImpl::local_comms_error(msg);
            socket_.abort();
            public_instance_->setErrorString(msg);
            status_ = Uploader::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
            // LCOV_EXCL_STOP
        }

        // We forward any QIODevice signals emitted by the socket to the public instance.
        connect(&socket_, &QIODevice::aboutToClose, public_instance_, &QIODevice::aboutToClose);
        connect(&socket_, &QIODevice::bytesWritten, public_instance_, &QIODevice::bytesWritten);
        connect(&socket_, &QIODevice::readChannelFinished, public_instance_, &QIODevice::readChannelFinished);
        connect(&socket_, &QIODevice::readyRead, public_instance_, &QIODevice::readyRead);

#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
        connect(&socket_, &QIODevice::channelBytesWritten, public_instance_, &QIODevice::channelBytesWritten);
        connect(&socket_, &QIODevice::channelReadyRead, public_instance_, &QIODevice::channelReadyRead);
#endif

        socket_.setSocketDescriptor(fd_.fileDescriptor(), QLocalSocket::ConnectedState, QIODevice::WriteOnly);
        status_ = Uploader::Status::Ready;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    process_error_ = [this](StorageError const& error)
    {
        // TODO: This does not set the method
        error_ = error;
        status_ = Uploader::Status::Error;
        socket_.abort();
        public_instance_->setErrorString(error.errorString());
        Q_EMIT public_instance_->statusChanged(status_);
    };

    handler_ = new Handler<QDBusPendingReply<QString, QDBusUnixFileDescriptor>>(this, reply, process_reply_, process_error_);
}

UploaderImpl::UploaderImpl(StorageError const& e)
    : status_(Uploader::Status::Error)
    , error_(e)
{
}

UploaderImpl::~UploaderImpl()
{
    switch (status_)
    {
        case Uploader::Status::Loading:
        case Uploader::Status::Finished:
        case Uploader::Status::Cancelled:
        case Uploader::Status::Error:
            break;
        case Uploader::Status::Ready:
            cancel();
            break;
        default:
            abort();  // Impossible.  // LCOV_EXCL_LINE
    }
}

bool UploaderImpl::isValid() const
{
    return status_ != Uploader::Status::Error && status_ != Uploader::Status::Cancelled;
}

Uploader::Status UploaderImpl::status() const
{
    return status_;
}

StorageError UploaderImpl::error() const
{
    return error_;
}

Item::ConflictPolicy UploaderImpl::policy() const
{
    return policy_;
}

qint64 UploaderImpl::sizeInBytes() const
{
    return size_in_bytes_;
}

Item UploaderImpl::item() const
{
    if (status_ != Uploader::Status::Finished)
    {
        return Item();
    }
    return Item(item_impl_);
}

void UploaderImpl::finishUpload()
{
    static QString const method = "Uploader::finishUpload()";

    // If we encountered an error earlier or were cancelled, or if finishUpload() was
    // called already, we ignore the call.
    if (status_ == Uploader::Status::Error || status_ == Uploader::Status::Cancelled || finalizing_)
    {
        return;
    }

    // Complain if we are asked to finalize while in the Loading or Finished state.
    if (status_ != Uploader::Ready)
    {
        QString msg = method + ": cannot finalize while Uploader is not in the Ready state";
        error_ = StorageErrorImpl::logic_error(msg);
        socket_.abort();
        public_instance_->setErrorString(msg);
        status_ = Uploader::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
        return;
    }

    auto runtime = item_impl_->runtime_impl();
    if (!runtime || !runtime->isValid())
    {
        QString msg = method + ": Runtime was destroyed previously";
        error_ = StorageErrorImpl::runtime_destroyed_error(msg);
        status_ = Uploader::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
        return;
    }

    finalizing_ = true;
    socket_.disconnectFromServer();
    auto reply = item_impl_->account_impl()->provider()->FinishUpload(upload_id_);

    auto process_reply = [this](decltype(reply)& r)
    {
        if (status_ == Uploader::Status::Cancelled || status_ == Uploader::Status::Error)
        {
            return;  // Don't transition to a final state more than once.
        }

        auto runtime = item_impl_->runtime_impl();
        if (!runtime || !runtime->isValid())
        {
            QString msg = method + ": Runtime was destroyed previously";
            error_ = StorageErrorImpl::runtime_destroyed_error(msg);
            socket_.abort();
            public_instance_->setErrorString(msg);
            status_ = Uploader::Status::Error;
            Q_EMIT public_instance_->statusChanged(status_);
            return;
        }

        auto metadata = r.value();
        try
        {
            validate_(metadata);
            item_impl_ = make_shared<ItemImpl>(metadata, item_impl_->account_impl());
            status_ = Uploader::Status::Finished;
        }
        catch (StorageError const& e)
        {
            // Bad metadata received from provider, validate_() or make_item() have logged it.
            // TODO: This does not set the method.
            error_ = e;
            status_ = Uploader::Status::Error;
        }
        Q_EMIT public_instance_->statusChanged(status_);
    };

    auto process_error = [this](StorageError const& error)
    {
        if (status_ != Uploader::Status::Ready)
        {
            return;  // Don't transition to a final state more than once.
        }

        // TODO: this doesn't set the method
        error_ = error;
        socket_.abort();
        public_instance_->setErrorString(error.errorString());
        status_ = Uploader::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
    };

    new Handler<void>(this, reply, process_reply, process_error);
}

void UploaderImpl::cancel()
{
    static QString const method = "Uploader::cancel()";

    // If we are in a final state already, ignore the call.
    if (   status_ == Uploader::Status::Error
        || status_ == Uploader::Status::Finished
        || status_ == Uploader::Status::Cancelled)
    {
        return;
    }
    auto runtime = item_impl_->runtime_impl();
    if (!runtime || !runtime->isValid())
    {
        QString msg = method + ": Runtime was destroyed previously";
        error_ = StorageErrorImpl::runtime_destroyed_error(msg);
        status_ = Uploader::Status::Error;
        Q_EMIT public_instance_->statusChanged(status_);
        return;
    }

    if (!upload_id_.isEmpty())
    {
        // We just send the cancel and ignore any reply because it is best-effort only.
        auto reply = item_impl_->account_impl()->provider()->CancelUpload(upload_id_);

        auto process_reply = [](decltype(reply)&)
        {
        };

        auto process_error = [](StorageError const&)
        {
        };

        new Handler<void>(this, reply, process_reply, process_error);
    }

    QString msg = method + ": upload was cancelled";
    error_ = StorageErrorImpl::cancelled_error(msg);
    socket_.abort();
    public_instance_->setErrorString(msg);
    status_ = Uploader::Status::Cancelled;
    Q_EMIT public_instance_->statusChanged(status_);
}

qint64 UploaderImpl::bytesAvailable() const
{
    return socket_.bytesAvailable();
}

qint64 UploaderImpl::bytesToWrite() const
{
    return socket_.bytesToWrite();
}

bool UploaderImpl::isSequential() const
{
    return socket_.isSequential();
}

bool UploaderImpl::waitForBytesWritten(int msecs)
{
    if (status_ == Uploader::Status::Loading)
    {
        // Unfortunately, QDBusPendingReply::waitForFinished() does not accept a timeout.
        // The next-best thing we can do is to simply wait without a timeout. The DBus
        // method will finish eventually, even though it might take a lot longer than msecs.
        handler_->wait_and_process_now();
    }
    if (flush_buffer() == -1)
    {
        return false;
    }
    return socket_.waitForBytesWritten(msecs);
}

bool UploaderImpl::waitForReadyRead(int msecs)
{
    return socket_.waitForReadyRead(msecs);
}

// LCOV_EXCL_START
// Never called by QIODevice because device is opened write-only.
qint64 UploaderImpl::readData(char* data, qint64 c)
{
    return socket_.read(data, c);
}
// LCOV_EXCL_STOP

qint64 UploaderImpl::writeData(char const* data, qint64 c)
{
    switch (status_)
    {
        case Uploader::Status::Loading:
        {
            // Client is writing before we have received the file descriptor from the provider.
            buffer_.append(data, c);
            return c;
        }
        case Uploader::Status::Ready:
        {
            if (flush_buffer() == -1)
            {
                return -1;
            }
            return socket_.write(data, c);
        }
        case Uploader::Status::Cancelled:
        case Uploader::Status::Finished:
        case Uploader::Status::Error:
        {
            return -1;  // Can't write to an already-finalized uploader.
        }
        default:
        {
            abort();  // Impossible  // LCOV_EXCL_LINE
        }
    }
    // NOTREACHED
}

Uploader* UploaderImpl::make_job(shared_ptr<ItemImpl> const& item_impl,
                                 QString const& method,
                                 QDBusPendingReply<QString, QDBusUnixFileDescriptor>& reply,
                                 std::function<void(storage::internal::ItemMetadata const&)> const& validate,
                                 Item::ConflictPolicy policy,
                                 qint64 size_in_bytes)
{
    unique_ptr<UploaderImpl> impl(new UploaderImpl(item_impl, method, reply, validate, policy, size_in_bytes));
    auto uploader = new Uploader(move(impl));
    uploader->open(QIODevice::WriteOnly);
    uploader->p_->public_instance_ = uploader;
    return uploader;
}

Uploader* UploaderImpl::make_job(StorageError const& e)
{
    unique_ptr<UploaderImpl> impl(new UploaderImpl(e));
    auto uploader = new Uploader(move(impl));
    uploader->open(QIODevice::WriteOnly);
    uploader->p_->public_instance_ = uploader;
    QMetaObject::invokeMethod(uploader,
                              "statusChanged",
                              Qt::QueuedConnection,
                              Q_ARG(unity::storage::qt::Uploader::Status, uploader->p_->status_));
    return uploader;
}

qint64 UploaderImpl::flush_buffer()
{
    qint64 bytes_written = 0;
    auto bytes_to_write = buffer_.size();
    if (bytes_to_write > 0)
    {
        auto bytes_written = socket_.write(buffer_.data(), bytes_to_write);
        if (bytes_written != bytes_to_write)
        {
            return -1;  // Not exactly detailed, but that's the best we can do.  // LCOV_EXCL_LINE
        }
        buffer_.resize(0);
    }
    return bytes_written;
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
