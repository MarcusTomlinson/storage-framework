#pragma once

#include <map>
#include <memory>
#include <string>

namespace unity
{

namespace storage
{

namespace client
{

namespace internal
{

class ItemImpl;

}  // namespace internal

class DestroyResult;
class MetadataResult;

/**
\brief Base class for files and directories.
*/

class Item
{
public:
    ~Item();
    Item(Item const&) = delete;
    Item& operator=(Item const&) = delete;
    Item(Item&&);
    Item& operator=(Item&&);

    typedef std::unique_ptr<Item> UPtr;

    /**
    \brief Returns the native identifier used by the provider.
    */
    std::string native_identity() const;

    /**
    \brief Returns the name of the file or directory.

    The returned name may not be the same as the name that was used to create the item because the provider
    may have changed it in some way (such as converting upper case characters to lower case).
    */
    std::string name() const;

    /**
    \brief Returns metadata for the item.
    \param metadata_callback The metadata functor can call MetadataResult::get_metadata() to retrieve the metadata.
    */
    void get_metadata(std::function<void(MetadataResult const&)> metadata_callback);

    /**
    \brief Permamently destroys the item.
    \warning Destroying a directory recursively destroys its contents.
    */
    void destroy(std::function<void(DestroyResult const&)> completion_callback);

private:
    Item();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
