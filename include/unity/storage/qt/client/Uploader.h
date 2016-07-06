#pragma once

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

class UploaderBase;

namespace local_client
{

class FileImpl;
class FolderImpl;

}  // namespace local_client

namespace remote_client
{

class UploaderImpl;

}  // namespace remote_client
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
    occurred, `result()` throws an exception. If the upload was cancelled, `result` throws CancelledException.
    Otherwise, it returns the File that was uploaded.

    Calling finish_upload() more than once is safe; subsequent calls do nothing and return the future
    that was returned by the first call.
    */
    QFuture<std::shared_ptr<File>> finish_upload();

    /**
    \brief Cancels an upload.

    Calling cancel() informs the provider that the upload is no longer needed. The provider
    will make a best-effort attempt to cancel the upload to the remote service.

    You can check whether the cancel was successfully sent by calling `waitForFinished()` on the returned future.
    If this does not throw an exception, the message was received and acted upon by the provider. However,
    successful completion does _not_ indicate that the upload was actually cancelled. (For example,
    the upload may have completed already by the time the provider received the cancel request, or the provider
    may not support cancellation.)

    Calling cancel() more than once, or calling cancel() after a call to finish_upload() is safe and does nothing.
    */
    QFuture<void> cancel();

private:
    Uploader(internal::UploaderBase*) UNITY_STORAGE_HIDDEN;

    std::shared_ptr<internal::UploaderBase> p_;

    friend class internal::local_client::FileImpl;
    friend class internal::local_client::FolderImpl;
    friend class internal::remote_client::UploaderImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
