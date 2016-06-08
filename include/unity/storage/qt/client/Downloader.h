#pragma once

#include <unity/storage/common.h>
#include <unity/storage/visibility.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

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
    the socket. Call `result()` on the returned future to check for errors. If an error
    occurred, `result()` throws an exception. Otherwise, it returns the transfer state to
    indicate whether the download finished normally or was cancelled.

    \warning Do not assume that a download completed successfully once you detect EOF on the socket.
    If something goes wrong during a download on the server side, the socket will return EOF
    for a partially-downloaded file.
    */
    QFuture<TransferState> finish_download();

    /**
    \brief Cancels a download.

    Calling cancel() informs the provider that the download is no longer needed. The provider
    will make a best-effort attempt to cancel the download from the remote service.

    Calling cancel() more than once, or calling cancel() after a call to finish_download() is safe and does nothing.
    */
    QFuture<void> cancel();

private:
    Downloader(internal::DownloaderBase*);

    std::shared_ptr<internal::DownloaderBase> p_;

    friend class internal::local_client::FileImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
