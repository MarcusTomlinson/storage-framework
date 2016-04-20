#pragma once

#include <memory>

namespace unity
{

namespace storage
{

namespace client
{

class UploadResult;

class Uploader
{
public:
    ~Uploader();
    Uploader(Uploader const&) = delete;
    Uploader& operator=(Uploader const&) = delete;
    Uploader(Uploader&&);
    Uploader& operator=(Uploader&&);

    typedef std::unique_ptr<Uploader> UPtr;

    /**
    \brief Returns a file descriptor that is open for writing.

    To upload the file contents, write to the returned file descriptor. The descriptor
    may not be able to accept arbitrarily large amounts of data without blocking. If the
    code that writes to the descriptor cannot block, use non-blocking writes. Once the
    contents of the file have been written, call close() to check for errors.

    If a write on the descriptor returns an error, this indicates a fatal error condition, and further
    writes on the descriptor will fail as well. In this case, a call to close() results in close_callback()
    being invoked as usual, and UploadResult::check_error() will return the error details
    via an exception.

    After an error, the file is in an indeterminate state.
    \return A file descriptor open for writing. If fd() is called after close() or cancel()
    have been called, the return value is `-1`.
    \throws TODO: All sorts of exceptions when things go wrong, list them.
    */
    int fd() const;

    /**
    \brief Finalizes the upload.

    Once you have written the file contents to the descriptor returned by fd(), you must call close(). The runtime
    calls closes the descriptor and calls close_callback once the state of the upload has been verified.
    Call UploadResult::check_error() from the callback to verify whether the upload completed successfully.
    \param close_callback The functor called by the runtime once the upload is finalized.
    The passed value must not be `nullptr`.
    */
    void close(std::function<void(UploadResult const&)> close_callback);

    /**
    \brief Cancels an upload.

    Calling cancel() informs the provider that the upload is no longer needed. The provider
    will make a best-effort attempt to cancel the upload to the remote service.

    Calling cancel() more than once, or calling cancel() after a call to close() is safe and does nothing.
    \throws TODO: All sorts of things.
    */
    void cancel();

private:
    Uploader();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
