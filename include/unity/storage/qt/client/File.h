#pragma once

#include <unity/storage/qt/client/Downloader.h>
#include <unity/storage/qt/client/Item.h>
#include <unity/storage/qt/client/Uploader.h>

namespace unity
{

namespace storage
{

namespace qt
{

namespace client
{

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

    typedef std::unique_ptr<File> UPtr;

    /**
    \brief Returns the size of the file in bytes.
    \throws DestroyedException if the file has been destroyed.
    */
    int64_t size() const;

    /**
    \brief Creates an uploader for the file.
    \param policy The conflict resolution policy.
    */
    QFuture<Uploader::UPtr> create_uploader(ConflictPolicy policy);

    /**
    \brief Creates a downloader for the file.
    */
    QFuture<Downloader::UPtr> create_downloader();
};

}  // namespace client

}  // namespace qt

}  // namespace storage

}  // namespace unity
