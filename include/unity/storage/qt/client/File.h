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

}  // namespace internal

enum class ConflictPolicy { error_if_conflict, overwrite };

/**
\brief Class that represents a file.

A file is a sequence of bytes.
*/
class File : public Item
{
public:
    ~File();
    File(File const&) = delete;
    File& operator=(File const&) = delete;
    File(File&&);
    File& operator=(File&&);

    typedef std::unique_ptr<File> SPtr;

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

    friend class FileImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
