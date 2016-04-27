#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

#include <memory>

namespace unity
{

namespace storage
{

namespace qt
{

namespace client
{

class Downloader
{
public:
    /**
    \brief Destroys the downloader.

    The destructor implicitly calls close() if it has not been called already.
    */
    ~Downloader();

    Downloader(Downloader const&) = delete;
    Downloader& operator=(Downloader const&) = delete;
    Downloader(Downloader&&);
    Downloader& operator=(Downloader&&);

    typedef std::unique_ptr<Downloader> UPtr;

    /**
    \brief Returns a file descriptor that is open for reading.

    To download the file contents, read from the returned file descriptor. The descriptor
    may not be able to deliver arbitrarily large amounts of data without blocking. If the
    code that reads from the descriptor cannot block, use non-blocking reads. Once the
    descriptor indicates EOF, call close() to check for errors.

    If a read on the descriptor returns an error, this indicates a fatal error condition, and further
    reads on the descriptor will fail as well.

    \return A file descriptor open for reading.
    */
    QFuture<int> fd() const;

    /**
    \brief Finalizes the download.

    Once the descriptor returned by fd() indicates EOF, you must call close().
    Call result() on the returned future to check for errors.

    \warning Do not assume that a download completed successfully once you detect EOF on the file descriptor.
    If something goes wrong during a download on the server side, the file descriptor will return EOF
    for a partially-downloaded file.
    */
    QFuture<void> close();

    /**
    \brief Cancels a download.

    Calling cancel() informs the provider that the download is no longer needed. The provider
    will make a best-effort attempt to cancel the download from the remote service.

    Calling cancel() more than once, or calling cancel() after a call to close() is safe and does nothing.
    */
    QFuture<void> cancel();

private:
    Downloader();
};

}  // namespace client

}  // namespace qt

}  // namespace storage

}  // namespace unity
