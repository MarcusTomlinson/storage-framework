#pragma once

#include "Downloader.h"
#include "Item.h"
#include "Uploader.h"

namespace unity
{

namespace storage
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
    \param ready_callback The runtime calls ready_callback once the uploader is ready to accept data.
    */
    void create_uploader(ConflictPolicy policy, std::function<void(Uploader::UPtr)> ready_callback);

    /**
    \brief Creates a downloader for the file.
    \param ready_callback The runtime calls ready_callback once the downloader is ready to deliver data.
    */
    void create_downloader(std::function<void(Downloader::UPtr)> ready_callback);
};

}  // namespace client

}  // namespace storage

}  // namespace unity
