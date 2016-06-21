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

class FileImpl;
class FolderImpl;
class UploadWorker;

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
    \param policy The conflict resolution policy.
    */
    QFuture<std::shared_ptr<Uploader>> create_uploader(ConflictPolicy policy);

    /**
    \brief Creates a downloader for the file.
    */
    QFuture<std::shared_ptr<Downloader>> create_downloader();

private:
    File(internal::FileImpl*);

    friend class internal::FileImpl;
    friend class internal::FolderImpl;
    friend class internal::ItemImpl;
    friend class internal::UploadWorker;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
