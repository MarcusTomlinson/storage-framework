/*
 * Copyright (C) 2017 Canonical Ltd
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

#include "LocalDownloadJob.h"

#include "LocalProvider.h"
#include "utils.h"
#include <unity/storage/internal/safe_strerror.h>
#include <unity/storage/provider/Exceptions.h>

using namespace unity::storage::provider;
using namespace std;

static int next_download_id = 0;

string const method = "download()";

LocalDownloadJob::LocalDownloadJob(shared_ptr<LocalProvider> const& provider,
                                   string const& item_id,
                                   string const& match_etag)
    : DownloadJob(to_string(++next_download_id))
    , provider_(provider)
    , item_id_(item_id)
{
    using namespace boost::filesystem;

    // Sanitize parameters.
    provider_->throw_if_not_valid(method, item_id_);
    try
    {
        auto st = status(item_id_);
        if (!is_regular_file(st))
        {
            throw LogicException(method + ": \"" + item_id_ + "\" is not a file");
        }
    }
    // LCOV_EXCL_START  // Too small a window to hit with a test.
    catch (filesystem_error const& e)
    {
        throw_storage_exception(method, e);
    }
    // LCOV_EXCL_STOP
    if (!match_etag.empty())
    {
        int64_t mtime = get_mtime_nsecs(method, item_id_);
        if (to_string(mtime) != match_etag)
        {
            throw ConflictException(method + ": ETag mismatch");
        }
    }

    // Make input file ready.
    QString filename = QString::fromStdString(item_id);
    file_.reset(new QFile(filename));
    if (!file_->open(QIODevice::ReadOnly))
    {
        throw_storage_exception(method,
                                ": cannot open \"" + item_id + "\": " + file_->errorString().toStdString(),
                                file_->error());
    }
    bytes_to_write_ = file_->size();

    // Make write socket ready.
    int dup_fd = dup(write_socket());
    if (dup_fd == -1)
    {
        // LCOV_EXCL_START
        string msg = "LocalDownloadJob(): dup() failed: " + unity::storage::internal::safe_strerror(errno);
        throw ResourceException(msg, errno);
        // LCOV_EXCL_STOP
    }
    write_socket_.setSocketDescriptor(dup_fd, QLocalSocket::ConnectedState, QIODevice::WriteOnly);
    connect(&write_socket_, &QIODevice::bytesWritten, this, &LocalDownloadJob::on_bytes_written);

    // Kick off the read-write cycle.
    QMetaObject::invokeMethod(this, "read_and_write_chunk", Qt::QueuedConnection);
}

LocalDownloadJob::~LocalDownloadJob() = default;

boost::future<void> LocalDownloadJob::cancel()
{
    disconnect(&write_socket_, nullptr, this, nullptr);
    write_socket_.abort();
    file_->close();
    return boost::make_ready_future();
}

boost::future<void> LocalDownloadJob::finish()
{
    if (bytes_to_write_ > 0)
    {
        auto file_size = file_->size();
        auto written = file_size - bytes_to_write_;
        string msg = "finish() method called too early, file \"" + item_id_ + "\" has size "
                     + to_string(file_size) + " but only " + to_string(written) + " bytes were consumed";
        cancel();
        return boost::make_exceptional_future<void>(LogicException(msg));
    }
    // LCOV_EXCL_START
    // Not reachable because we call report_complete() in read_and_write_chunk().
    return boost::make_ready_future();
    // LCOV_EXCL_STOP
}

void LocalDownloadJob::on_bytes_written(qint64 bytes)
{
    bytes_to_write_ -= bytes;
    assert(bytes_to_write_ >= 0);
    read_and_write_chunk();
}

void LocalDownloadJob::read_and_write_chunk()
{
    static qint64 constexpr READ_SIZE = 64 * 1024;

    if (bytes_to_write_ == 0)
    {
        file_->close();
        write_socket_.close();
        report_complete();
        return;
    }

    QByteArray buf;
    buf.resize(READ_SIZE);
    auto bytes_read = file_->read(buf.data(), buf.size());
    try
    {
        if (bytes_read == -1)
        {
            // LCOV_EXCL_START
            string msg = string("\"") + item_id_ + "\": read error: " + file_->errorString().toStdString();
            throw_storage_exception(method, msg, file_->error());
            // LCOV_EXCL_STOP
        }
        buf.resize(bytes_read);

        auto bytes_written = write_socket_.write(buf);
        if (bytes_written == -1)
        {
            // LCOV_EXCL_START
            string msg = string("\"") + item_id_ + "\": socket error: " + write_socket_.errorString().toStdString();
            throw_storage_exception(method, msg, write_socket_.error());
            // LCOV_EXCL_STOP
        }
        else if (bytes_written != bytes_read)
        {
            // LCOV_EXCL_START
            string msg = string("\"") + item_id_ + "\": socket write error, requested " + to_string(bytes_read)
                         + " B, but wrote only " + to_string(bytes_written) + " B.";
            throw_storage_exception(method, msg, QLocalSocket::UnknownSocketError);
            // LCOV_EXCL_STOP
        }
    }
    // LCOV_EXCL_START
    catch (std::exception const&)
    {
        write_socket_.abort();
        file_->close();
        report_error(current_exception());
    }
    // LCOV_EXCL_STOP
}
