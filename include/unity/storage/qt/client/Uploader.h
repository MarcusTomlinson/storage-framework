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
    writes on the descriptor will fail as well.

    After an error, the file is in an indeterminate state.
    \return A file descriptor open for writing.
    */
    QFuture<int> fd() const;

    /**
    \brief Finalizes the upload.

    Once you have written the file contents to the descriptor returned by fd(), you must call close().
    Call result() on the returned future to check for errors.
    */
    QFuture<void> close();

    /**
    \brief Cancels an upload.

    Calling cancel() informs the provider that the upload is no longer needed. The provider
    will make a best-effort attempt to cancel the upload to the remote service.

    Calling cancel() more than once, or calling cancel() after a call to close() is safe and does nothing.
    */
    QFuture<void> cancel();

private:
    Uploader();
};

}  // namespace client

}  // namespace qt

}  // namespace storage

}  // namespace unity
