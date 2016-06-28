#pragma once

#include <unity/storage/qt/client/Item.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Downloader;
class Uploader;
namespace internal
{

class FileBase;

namespace local_client
{

class FileImpl;

}  // namespace local_client

namespace remote_client
{

class FileImpl;

}  // namespace remotelocal_client
}  // namespace internal

/**
\brief Class that represents a file.

A file is a sequence of bytes.
*/
class UNITY_STORAGE_EXPORT File final : public Item
{
public:
    /// @cond
    virtual ~File();
    /// @endcond

    File(File&&);
    File& operator=(File&&);

    /**
    \brief Convenience type definition.
    */
    typedef std::shared_ptr<File> SPtr;

    /**
    \brief Returns the size of the file in bytes.
    \throws DestroyedException if the file has been destroyed.
    */
    int64_t size() const;

    /**
    \brief Creates an uploader for the file.
    \param policy The conflict resolution policy. If set to ConflictPolicy::overwrite,
    the contents of the file will be overwritten even if the file was modified
    after this File instance was retrieved. Otherwise, if set to ConflictPolicy::error_if_conflict,
    an attempt to retrieve the File instance from the future returned by Uploader::finish_upload()
    throws ConflictException.
    */
    QFuture<std::shared_ptr<Uploader>> create_uploader(ConflictPolicy policy);

    /**
    \brief Creates a downloader for the file.
    */
    QFuture<std::shared_ptr<Downloader>> create_downloader();

private:
    File(internal::FileBase*);

    friend class internal::local_client::FileImpl;
    friend class internal::remote_client::FileImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
