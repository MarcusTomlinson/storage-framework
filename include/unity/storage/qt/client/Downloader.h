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

#include <unity/storage/common.h>
#include <unity/storage/visibility.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop
#include <QLocalSocket>

#include <memory>

class QLocalSocket;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class File;

namespace internal
{

class DownloaderBase;

namespace local_client
{

class FileImpl;

}  // namespace local_client

namespace remote_client
{

class DownloaderImpl;

}  // namespace remote_client
}  // namespace internal

class UNITY_STORAGE_EXPORT Downloader final
{
public:
    /**
    \brief Destroys the downloader.

    The destructor implicitly calls cancel() if it has not been called already.
    */
    ~Downloader();

    Downloader(Downloader&&);
    Downloader& operator=(Downloader&&);

    /**
    \brief Convenience type definition.
    */
    typedef std::shared_ptr<Downloader> SPtr;

    /**
    \brief Returns the file for this downloader.
    */
    std::shared_ptr<File> file() const;

    /**
    \brief Returns a socket that is open for reading.

    To download the file contents, read from the returned socket.
    \return A socket open for reading.
    */
    std::shared_ptr<QLocalSocket> socket() const;

    /**
    \brief Finalizes the download.

    Once the returned socket indicates EOF, you must call finish_download(), which closes
    the socket. Call `waitForFinished()` on the returned future to check for errors. If an error
    occurred, `waitForFinished()` throws an exception. If the download was cancelled, `waitForFinished()` throws
    CancelledException.

    \warning Do not assume that a download completed successfully once you detect EOF on the socket.
    If something goes wrong during a download on the server side, the socket will return EOF
    for a partially-downloaded file.
    */
    QFuture<void> finish_download();

    /**
    \brief Cancels a download.

    Calling cancel() informs the provider that the download is no longer needed. The provider
    will make a best-effort attempt to cancel the download from the remote service.

    You can check whether the cancel was successfully sent by calling `waitForFinished()` on the returned future.
    If this does not throw an exception, the message was received and acted upon by the provider. However,
    successful completion does _not_ indicate that the download was actually cancelled. (For example,
    the download may have completed already by the time the provider received the cancel request, or the provider
    may not support cancellation.)

    Calling cancel() more than once, or calling cancel() after a call to finish_download() is safe and does nothing.
    */
    QFuture<void> cancel();

private:
    Downloader(internal::DownloaderBase*) UNITY_STORAGE_HIDDEN;

    std::shared_ptr<internal::DownloaderBase> p_;

    friend class internal::local_client::FileImpl;
    friend class internal::remote_client::DownloaderImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
