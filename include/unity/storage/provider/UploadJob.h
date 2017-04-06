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
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#pragma once

#include <unity/storage/visibility.h>

#include <boost/thread/future.hpp>
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

struct Item;

namespace internal
{
class PendingJobs;
class ProviderInterface;
class UploadJobImpl;
}

class TempfileUploadJob;

/**
\brief Abstract base class for upload implementations.

When the runtime calls ProviderBase::update() or ProviderBase::create_file(), you must return
a new uploader instance that derives from UploadJob. Your implementation is responsible
for retrieving the file's data from the upload socket and writing it to
the corresponding file in the cloud provider.

You can implement your uploader
any way you wish, such as by running the upload in a separate thread, or
by using async I/O driven by the runtime's (or any other) event loop.

The runtime invokes all methods on the uploader from the main thread.
*/

class UNITY_STORAGE_EXPORT UploadJob
{
public:
    /**
    \brief Construct an uploader.
    \param upload_id An identifier for this particular upload. You can use any non-empty string,
    as long as it is unique among all uploads that are in progress within the corresponding account.
    (A simple incrementing counter will work fine, or you can use the upload identifier you receive
    from the cloud service.)
    The runtime uses the <code>upload_id</code> to distinguish different uploads that may be
    in progress concurrently, and it ensures that each ID can be used only by its corresponding client.
    */
    UploadJob(std::string const& upload_id);
    virtual ~UploadJob();

    /**
    \brief Returns the upload ID.
    \return The value of the <code>upload_id</code> parameter that was passed to the constructor.
    */
    std::string const& upload_id() const;

    /**
    \brief Returns the socket to read the file contents from.
    \return A socket that is open for reading. You must read the file contents from this socket (which is
    connected to the client by the runtime).
    */
    int read_socket() const;

    /**
    \brief Informs the runtime that an upload encountered an error.

    This method makes it unnecessary to wait for the runtime to call finish() in order to confirm
    whether an upload completed successfully. You can call report_error() as soon as you encounter an error
    during an upload (such as losing the connection to the cloud provider, or getting an error when reading
    from the upload socket). This can be convenient if an upload runs in a separate thread or event loop
    because there is no need to store success/error state for use in finish().

    You can call report_error() from an arbitrary thread.

    If you call report_error(), the runtime guarantees that neither finish() nor cancel() will be called, so
    you must reclaim any resources associated with the upload before calling report_error().

    \param storage_exception You <i>must</i> pass a StorageException to indicate the reason for the failure.
    \see finish()
    */
    void report_error(std::exception_ptr storage_exception);

    /**
    \brief Cancel this upload.

    The runtime calls this method when a client explicitly cancels an upload or crashes.
    Your implementation should reclaim all resources that are used by the upload. In particular,
    you should stop reading any more data and close the upload socket. In addition,
    you should reclaim any resources (such as open connections) that are associated
    with the upload to the cloud provider (possibly after informing the cloud provider of
    the cancellation).

    The runtime guarantees that cancel() will be called only once, and that finish() will not be called
    if cancel() was called.

    If any errors are encountered, you <i>must</i> report them by returning a future that stores
    a StorageException. Do <i>not</i> call report_error() from inside cancel().

    \return A future that becomes ready (or contains a StorageException) once cancellation is complete.
    */
    virtual boost::future<void> cancel() = 0;

    /**
    \brief Finalize this upload.

    The runtime calls this method when a client finishes an upload. Your implementation <i>must</i> verify
    that it has successfully read <i>and</i> written the <i>exact</i> number of bytes that were passed
    to ProviderBase::update() or ProviderBase::create_file() before making the future ready.
    If too few or too many bytes were sent by the client, it <i>must</i> store a LogicException in the future.
    It must also verify that the cloud provider has received <i>all</i> of the file's data; otherwise,
    the file may end up being partially written in the cloud provider.

    Note that finish() may be called while there is still buffered data that remains to be read from the
    upload socket, so you must take care to drain the socket of any remaining data before checking that the
    actual number bytes uploaded by the client matches the expected number of bytes.

    You should close the upload socket and reclaim any resources (such as open
    connections) that are associated with the upload from the cloud provider.

    The runtime guarantees that finish() will be called only once, and that cancel() will not be called
    if finish() was called.

    If any errors are encountered, you <i>must</i> report them by returning a future that stores
    a StorageException. Do <i>not</i> call report_error() from inside finish().

    \return A future that becomes ready and contains the metadata for the file (or contains a StorageException)
    once the upload is complete.
    \see report_error(), TempfileUploadJob
    */
    virtual boost::future<Item> finish() = 0;

private:
    UploadJob(internal::UploadJobImpl *p) UNITY_STORAGE_HIDDEN;
    internal::UploadJobImpl *p_ = nullptr;

    friend class internal::PendingJobs;
    friend class internal::ProviderInterface;
    friend class TempfileUploadJob;
};

}
}
}
