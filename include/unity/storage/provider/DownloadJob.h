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
#include <stdexcept>
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{
class DownloadJobImpl;
class PendingJobs;
class ProviderInterface;
}

/**
\brief Abstract base class for download implementations.

When the runtime calls ProviderBase::download(), you must return
a new downloader instance that derives from DownloadJob. Your implementation is responsible
for retrieving the file's data from the cloud provider and writing it to
the socket provided by the runtime.

You can implement your downloader
any way you wish, such as by running the download in a separate thread, or
by using async I/O driven by the runtime's (or any other) event loop.

The runtime invokes all methods on the downloader from the main thread.
*/

class UNITY_STORAGE_EXPORT DownloadJob
{
public:
    /**
    \brief Construct a downloader.
    \param download_id An identifier for this particular download. You can use any non-empty string,
    as long as it is unique among all downloads that are in progress within this provider.
    The runtime uses the <code>download_id</code> to distinguish different downloads that may be
    in progress concurrently.
    */
    DownloadJob(std::string const& download_id);
    virtual ~DownloadJob();

    /**
    \brief Returns the download ID.
    \return The value of the <code>download_id</code> parameter that was passed to the constructor.
    */
    std::string const& download_id() const;

    /**
    \brief Returns the socket to write the file contents to.
    \return A socket that is open for writing. You must write the file contents to this socket (which is
    connected to the client by the runtime).
    */
    int write_socket() const;

    /**
    \brief Informs the runtime that a download completed successfully.

    This method makes it unnecessary to wait for the runtime to call finish() in order to confirm
    whether a download completed successfully. You can call report_complete() as soon as you have
    written all of a file's data to the download socket and closed it successfully. This can be convenient
    if a download runs in a separate thread or event loop because there is no need to store success/error state
    for use in finish().

    You can call report_complete() from an arbitrary thread.

    If you call report_complete(), the runtime guarantees that neither finish() nor cancel() will be called, so
    you must reclaim any resources associated with the download before calling report_complete().
    \see report_error(), finish()
    */
    void report_complete();

    /**
    \brief Informs the runtime that a download encountered an error.

    As for report_complete(), you can call report_error() as soon as you encounter an error during a download
    (such as losing the connection to the cloud provider, or getting an error when writing to the download
    socket).

    You can call report_error() from an arbitrary thread.

    If you call report_error(), the runtime guarantees that neither finish() nor cancel() will be called, so
    you must reclaim any resources associated with the download before calling report_error().

    \param storage_exception You <i>must</i> pass a StorageException to indicate the reason for the failure. Be as
    detailed in the error message as possible and include any details you receive from the cloud provider, as well
    as the identity of the file. Without this, it may be impossible to diagnose the problem from log files.
    \see report_complete(), finish()
    */
    void report_error(std::exception_ptr storage_exception);

    /**
    \brief Cancel this download.

    The runtime calls this method when a client explicitly cancels a download or crashes.
    Your implementation should reclaim all resources that are used by the download. In particular,
    you should stop writing any more data and close the download socket. In addition,
    you should reclaim any resources (such as open connections) that are associated
    with the download from the cloud provider (possibly after informing the cloud provider of
    the cancellation).

    The runtime guarantees that cancel() will be called only once, and that finish() will not be called
    if cancel() was called.

    If any errors are encountered, you <i>must</i> report them by returning a future that stores
    a StorageException. Do <i>not</i> call report_error() from inside cancel().

    \return A future that becomes ready (or contains a StorageException) once cancellation is complete.
    */
    virtual boost::future<void> cancel() = 0;

    /**
    \brief Finalize this download.

    The runtime calls this method when a client finishes a download. Your implementation <i>must</i> verify
    at this point that it has successfully written <i>all</i> of the file's data at this point.
    If not, it <i>must</i> store a LogicException
    in the returned future. (This is essential to make sure that the client can distinguish orderly socket
    closure from disorderly closure and has not received partial data for the file.)

    You should close the download socket and reclaim any resources (such as open
    connections) that are associated with the download from the cloud provider.

    The runtime guarantees that finish() will be called only once, and that cancel() will not be called
    if finish() was called.

    If any errors are encountered, you <i>must</i> report them by returning a future that stores
    a StorageException. Do <i>not</i> call report_error() from inside finish().

    Do <i>not</i> call report_complete() from inside finish().

    \return A future that becomes ready (or contains a StorageException) once the download is complete.
    \see report_complete(), report_error()
    */
    virtual boost::future<void> finish() = 0;

private:
    DownloadJob(internal::DownloadJobImpl *p) UNITY_STORAGE_HIDDEN;
    internal::DownloadJobImpl *p_ = nullptr;

    friend class internal::PendingJobs;
    friend class internal::ProviderInterface;
};

}
}
}
