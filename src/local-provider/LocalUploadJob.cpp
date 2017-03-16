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

#include "LocalUploadJob.h"

#include "LocalProvider.h"
#include "utils.h"
#include <unity/storage/internal/safe_strerror.h>
#include <unity/storage/provider/Exceptions.h>

#include <fcntl.h>

using namespace unity::storage::provider;
using namespace std;

static int next_upload_id = 0;

LocalUploadJob::LocalUploadJob(shared_ptr<LocalProvider> const& provider, int64_t size, const string& method)
    : UploadJob(to_string(++next_upload_id))
    , provider_(provider)
    , size_(size)
    , bytes_to_write_(size)
    , method_(method)
    , state_(in_progress)
    , tmp_fd_([](int fd){ if (fd != -1) ::close(fd); })
{
}

LocalUploadJob::LocalUploadJob(shared_ptr<LocalProvider> const& provider,
                               string const& parent_id,
                               string const& name,
                               int64_t size,
                               bool allow_overwrite)
    : LocalUploadJob(provider, size, "create_file()")
{
    using namespace boost::filesystem;

    parent_id_ = parent_id;
    allow_overwrite_ = allow_overwrite;

    provider_->throw_if_not_valid(method_, parent_id);

    auto sanitized_name = sanitize(method_, name);
    path p = parent_id;
    p /= sanitized_name;
    item_id_ = p.native();
    if (!allow_overwrite && exists(item_id_))
    {
        string msg = method_ + ": \"" + item_id_ + "\" exists already";
        throw ExistsException(msg, item_id_, sanitized_name.native());
    }

    prepare_channels();
}

LocalUploadJob::LocalUploadJob(shared_ptr<LocalProvider> const& provider,
                               string const& item_id,
                               int64_t size,
                               string const& old_etag)
    : LocalUploadJob(provider, size, "update()")
{
    using namespace boost::filesystem;

    item_id_ = item_id;
    provider_->throw_if_not_valid(method_, item_id);
    try
    {
        auto st = status(item_id);
        if (!is_regular_file(st))
        {
            throw InvalidArgumentException(method_ + ": \"" + item_id + "\" is not a file");
        }
    }
    // LCOV_EXCL_START
    catch (filesystem_error const& e)
    {
        // The call to status could throw if the file is unlinked immediately
        // after the call to throw_if_not_valid.
        throw_storage_exception(method_, e);
    }
    // LCOV_EXCL_STOP
    if (!old_etag.empty())
    {
        int64_t mtime = get_mtime_nsecs(method_, item_id);
        if (to_string(mtime) != old_etag)
        {
            throw ConflictException(method_ + ": etag mismatch");
        }
    }
    old_etag_ = old_etag;

    prepare_channels();
}

LocalUploadJob::~LocalUploadJob() = default;

void LocalUploadJob::prepare_channels()
{
    using namespace boost::filesystem;

    // Open tmp file for writing.
    auto parent_path = path(item_id_).parent_path();
    tmp_fd_.reset(open(parent_path.native().c_str(), O_TMPFILE | O_WRONLY, 0600));
    if (tmp_fd_.get() == -1)
    {
        // Some kernels on the phones don't support O_TMPFILE and return various errno values when this fails.
        // So, if anything at all goes wrong, we fall back on conventional temp file creation and
        // produce a hard error if that doesn't work either.
        // Note that, in this case, the temp file retains its name in the file system. Not nice because,
        // if this process dies at the wrong moment, we leave the temp file behind.
        use_linkat_ = false;
        string tmpfile = parent_path.native() + "/" + TMPFILE_PREFIX + "-%%%%-%%%%-%%%%-%%%%";
        tmp_fd_.reset(mkstemp(const_cast<char*>(tmpfile.data())));
        if (tmp_fd_.get() == -1)
        {
            string msg = method_ + ": cannot create temp file \"" + tmpfile + "\": "
                         + unity::storage::internal::safe_strerror(errno);
            throw ResourceException(msg, errno);
        }
        // LCOV_EXCL_START
        file_.reset(new QFile(QString::fromStdString(tmpfile)));
        file_->open(QIODevice::WriteOnly);
        // LCOV_EXCL_STOP
    }
    else
    {
        use_linkat_ = true;
        file_.reset(new QFile);
        file_->open(tmp_fd_.get(), QIODevice::WriteOnly, QFileDevice::DontCloseHandle);
    }

    // Make read socket ready.
    int dup_fd = dup(read_socket());
    if (dup_fd == -1)
    {
        // LCOV_EXCL_START
        string msg = method_ + ": dup() failed: " + unity::storage::internal::safe_strerror(errno);
        throw ResourceException(msg, errno);
        // LCOV_EXCL_STOP
    }
    read_socket_.setSocketDescriptor(dup_fd, QLocalSocket::ConnectedState, QIODevice::ReadOnly);
    connect(&read_socket_, &QLocalSocket::readyRead, this, &LocalUploadJob::on_bytes_ready);
    connect(&read_socket_, &QIODevice::readChannelFinished, this, &LocalUploadJob::on_read_channel_finished);
}

boost::future<void> LocalUploadJob::cancel()
{
    if (state_ == in_progress)
    {
        abort_upload();
    }
    return boost::make_ready_future();
}

boost::future<Item> LocalUploadJob::finish()
{
    on_bytes_ready();  // Read any remaining unread buffered data.

    if (bytes_to_write_ > 0)
    {
        string msg = "finish() method called too early, size was given as "
                     + to_string(size_) + " but only "
                     + to_string(size_ - bytes_to_write_) + " bytes were received";
        return boost::make_exceptional_future<Item>(LogicException(msg));
    }

    // We are committed to finishing successfully or with an error now.
    state_ = finished;

    try
    {
        // We check again for an etag mismatch or overwrite, in case the file was updated after the upload started.
        if (!parent_id_.empty())
        {
            // create_file()
            if (!allow_overwrite_ && boost::filesystem::exists(item_id_))
            {
                string msg = method_ + ": \"" + item_id_ + "\" exists already";
                boost::filesystem::path(item_id_).filename().native();
                BOOST_THROW_EXCEPTION(
                    ExistsException(msg, item_id_, boost::filesystem::path(item_id_).filename().native()));
            }
        }
        else if (!old_etag_.empty())
        {
            // update()
            int64_t mtime = get_mtime_nsecs(method_, item_id_);
            if (to_string(mtime) != old_etag_)
            {
                BOOST_THROW_EXCEPTION(ConflictException(method_ + ": etag mismatch"));
            }
        }

        if (!file_->flush())  // Make sure that all buffered data is written.
        {
            // LCOV_EXCL_START
            string msg = "finish(): cannot flush output file: " + file_->errorString().toStdString();
            throw_storage_exception("finish()", msg, file_->error());
            // LCOV_EXCL_STOP
        }

        // Link the anonymous tmp file into the file system.
        using namespace unity::storage::internal;

        if (use_linkat_)
        {
            auto old_path = string("/proc/self/fd/") + std::to_string(tmp_fd_.get());
            ::unlink(item_id_.c_str());  // linkat() will not remove existing file: http://lwn.net/Articles/559969/
            if (linkat(-1, old_path.c_str(), tmp_fd_.get(), item_id_.c_str(), AT_SYMLINK_FOLLOW) == -1)
            {
                // LCOV_EXCL_START
                string msg = "finish(): linkat \"" + old_path + "\" to \"" + item_id_ + "\" failed: "
                             + safe_strerror(errno);
                BOOST_THROW_EXCEPTION(ResourceException(msg, errno));
                // LCOV_EXCL_STOP
            }
        }
        else
        {
            // LCOV_EXCL_START
            auto old_path = file_->fileName().toStdString();
            if (rename(old_path.c_str(), item_id_.c_str()) == -1)
            {
                string msg = "finish(): rename \"" + old_path + "\" to \"" + item_id_ + "\" failed: "
                             + safe_strerror(errno);
                BOOST_THROW_EXCEPTION(ResourceException(msg, errno));
            }
            // LCOV_EXCL_STOP
        }

        file_->close();
        read_socket_.close();

        auto st = boost::filesystem::status(item_id_);
        return boost::make_ready_future<Item>(provider_->make_item(method_, item_id_, st));
    }
    catch (StorageException const&)
    {
        return boost::make_exceptional_future<Item>(boost::current_exception());
    }
    // LCOV_EXCL_START
    catch (boost::filesystem::filesystem_error const& e)
    {
        try
        {
            throw_storage_exception("finish()", e);
        }
        catch (StorageException const&)
        {
            return boost::make_exceptional_future<Item>(boost::current_exception());
        }
    }
    catch (std::exception const& e)
    {
        return boost::make_exceptional_future<Item>(UnknownException(e.what()));
    }
    // LCOV_EXCL_STOP
}

void LocalUploadJob::on_bytes_ready()
{
    if (bytes_to_write_ < 0)
    {
        return;  // LCOV_EXCL_LINE  // We received too many bytes earlier.
    }

    try
    {
        auto buf = read_socket_.readAll();
        if (buf.size() != 0)
        {
            bytes_to_write_ -= buf.size();
            if (bytes_to_write_ < 0)
            {
                string msg = method_ + ": received more than the expected number (" + to_string(size_) + ") of bytes";
                throw LogicException(msg);
            }
            auto bytes_written = file_->write(buf);
            if (bytes_written == -1)
            {
                // LCOV_EXCL_START
                string msg = "write error: " + file_->errorString().toStdString();
                throw_storage_exception(method_, msg, file_->error());
                // LCOV_EXCL_STOP
            }
            else if (bytes_written != buf.size())
            {
                // LCOV_EXCL_START
                string msg = "write error, requested " + to_string(buf.size()) + " B, but wrote only "
                             + to_string(bytes_written) + " B.";
                throw_storage_exception(method_, msg, QFileDevice::FatalError);
                // LCOV_EXCL_STOP
            }
        }
    }
    catch (std::exception const&)
    {
        abort_upload();
        report_error(current_exception());
    }
}

void LocalUploadJob::on_read_channel_finished()
{
    on_bytes_ready();  // In case there s still buffered data to be read.
}

void LocalUploadJob::abort_upload()
{
    state_ = cancelled;
    disconnect(&read_socket_, nullptr, this, nullptr);
    read_socket_.abort();
    file_->close();
    if (!use_linkat_)
    {
        // LCOV_EXCL_START
        string filename = file_->fileName().toStdString();
        ::unlink(filename.c_str());  // Don't leave any temp file behind.
        // LCOV_EXCL_STOP
    }
    bytes_to_write_ = 0;
}
