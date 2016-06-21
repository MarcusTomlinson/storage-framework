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

class FileImpl;
class UploaderImpl;

}  // namespace internal

class UNITY_STORAGE_EXPORT Uploader final
{
public:
    /**
    \brief Destroys the uploader.

    The destructor implicitly calls cancel() if it has not been called already.
    */
    ~Uploader();

    Uploader(Uploader&&);
    Uploader& operator=(Uploader&&);

    /**
    \brief Convenience type definition.
    */
    typedef std::shared_ptr<Uploader> SPtr;

    /**
    \brief Returns the file for this uploader.
    */
    std::shared_ptr<File> file() const;

    /**
    \brief Returns a socket that is open for writing.

    To upload the file contents, write to the returned socket.
    If an operation on the socket returns an error, the file is in an indeterminate state.

    \return A socket open for writing.
    */
    std::shared_ptr<QLocalSocket> socket() const;

    /**
    \brief Finalizes the upload.

    Once you have written the file contents to the socket returned by socket(), you must call finish_upload(),
    which closes the socket. Call `result()` on the returned future to check for errors. If an error
    occurred, `result()` throws an exception. Otherwise, it returns the transfer state to
    indicate whether the upload finished normally or was cancelled.

    Calling finish_upload() more than once is safe; subsequent calls do nothing and return the future
    that was returned by the first call.
    */
    QFuture<TransferState> finish_upload();

    /**
    \brief Cancels an upload.

    Calling cancel() informs the provider that the upload is no longer needed. The provider
    will make a best-effort attempt to cancel the upload to the remote service.

    Calling cancel() more than once, or calling cancel() after a call to finish_upload() is safe and does nothing.
    */
    QFuture<void> cancel();

private:
    Uploader(internal::UploaderImpl*);

    std::shared_ptr<internal::UploaderImpl> p_;

    friend class internal::FileImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity