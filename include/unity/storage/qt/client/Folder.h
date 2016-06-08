#pragma once

#include <unity/storage/qt/client/File.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{

class FolderBase;

namespace local_client
{

class FolderImpl;
class ItemImpl;

}  // namespace local_client

namespace remote_client
{

class FolderImpl;
class ItemImpl;

}  // namespace local_client
}  // namespace internal

/**
\brief Class that represents a folder.

A folder is an unordered set of files and/or folders.
*/
class UNITY_STORAGE_EXPORT Folder : public Item
{
public:
    /// @cond
    virtual ~Folder();
    /// @endcond

    Folder(Folder&&);
    Folder& operator=(Folder&&);

    typedef std::shared_ptr<Folder> SPtr;

    /**
    \brief Returns the contents of a folder.
    \return A vector of items or, if this folder is empty,
    an empty vector. If there is a large number of items,
    the returned future may become ready
    more than once. (See QFutureWatcher for more information.)
    */
    QFuture<QVector<Item::SPtr>> list() const;

    /**
    \brief Returns the item within this folder with the given name.
    \return The item. If no such item exists, retrieving the result
    from the future throws an exception.
    */
    QFuture<Item::SPtr> lookup(QString const& name) const;

    /**
    \brief Creates a new folder with the current folder as the parent.
    \param name The name of the new folder. Note that the actual name may be changed
    by the provider; call Item::name() once the folder is created to get its actual name.
    // TODO: Explain issues with metacharacters.
    \return The new folder.
    */
    QFuture<Folder::SPtr> create_folder(QString const& name);

    /**
    \brief Creates a new empty file with the current folder as the parent.
    \param name The name of the new file. Note that the actual name may be changed
    by the provider; call Item::name() once the file is created to get its actual name.
    // TODO: Explain issues with metacharacters.
    */
    QFuture<std::shared_ptr<Uploader>> create_file(QString const& name);

protected:
    Folder(internal::FolderBase*);

    friend class internal::local_client::FolderImpl;
    friend class internal::local_client::ItemImpl;
    friend class internal::remote_client::FolderImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
