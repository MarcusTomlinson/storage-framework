#pragma once

namespace unity
{

namespace storage
{

namespace client
{

class DownloadResult;

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

    \return A file descriptor open for reading. If fd() is called after close() or cancel()
    have been called, the return value is `-1`.
    \throws TODO: All sorts of exceptions when things go wrong, list them.
    */
    int fd() const;

    /**
    \brief Finalizes the download.

    Once the descriptor returned by fd() indicates EOF, you must call close(). The runtime
    closes the descriptor and calls close_callback once the state of the download has been verified.
    Call DownloadResult::check_error() from the callback to verify whether the download completed successfully.

    If a read on the descriptor returns an error, this indicates a fatal error condition, and further
    reads on the descriptor will fail as well. In this case, a call to close() results in close_callback()
    being invoked as usual, and DownloadResult::check_error() will return the error details
    via an exception.

    \param close_callback The functor called by the runtime once the download is finalized.
    The passed value must not be `nullptr`.
    \warning Do not assume that a download completed successfully once you detect EOF on the file descriptor.
    If something goes wrong during a download on the server side, the file descriptor will return EOF
    for a partially-downloaded file.
    */
    void close(std::function<void(DownloadResult const&)> close_callback);

    /**
    \brief Cancels a download.

    Calling cancel() informs the provider that the download is no longer needed. The provider
    will make a best-effort attempt to cancel the download from the remote service.

    Calling cancel() more than once, or calling cancel() after a call to close() is safe and does nothing.
    \throws TODO: All sorts of things.
    */
    void cancel();

private:
    Downloader();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
