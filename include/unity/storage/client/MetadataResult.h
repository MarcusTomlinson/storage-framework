#pragma once

#include "Variant.h"

#include <chrono>

namespace unity
{

namespace storage
{

namespace client
{

class MetadataResult
{
public:
    ~MetadataResult();
    MetadataResult(MetadataResult const&) = delete;
    MetadataResult& operator=(MetadataResult const&) = delete;
    MetadataResult(MetadataResult&&) = delete;
    MetadataResult& operator=(MetadataResult&&) = delete;

    VariantMap get_metadata() const;

    /**
    \brief Returns the time at which the item was last modified.
    \throws DestroyedException if the item has been destroyed.
    */
    std::chrono::system_clock::time_point modified_time() const;

    /**
    \brief Returns the mime type of the item.
    \returns For directories, the mime type is `inode/directory`. If the mime type is unknown, the returned string is empty.
    \throws DestroyedException if the item has been destroyed.
    */
    std::string mime_type() const;


private:
    MetadataResult();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
