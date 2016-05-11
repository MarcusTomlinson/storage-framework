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

class FolderImpl;

}  // namespace internal

/**
\brief Class that represents a folder.

A folder is an unordered set of files and/or folders. The names (as returned by Item::name())
of the members of a folder are unique.
*/
class Folder : public Item
{
public:
    ~Folder();
    Folder(Folder const&) = delete;
    Folder& operator=(Folder const&) = delete;
    Folder(Folder&&);
    Folder& operator=(Folder&&);

    typedef std::unique_ptr<Folder> SPtr;

    /**
    \brief Returns (possibly partial) contents of a folder.
    \return A vector of items or, if this folder is empty,
    an empty vector. Not all items may be returned by a particular call; if
    there is a large number of items, several calls may be required to
    obtain all of them. An empty vector indicates end-of-iteration.
    */
    QFuture<QVector<Item::SPtr>> list() const;

    /**
    \brief Returns the item within this folder with the given name.
    \return The item. If no such item exists, retrieving the result
    from the future throws an exception.
    */
    QFuture<QVector<Item::SPtr>> lookup(QString const& name) const;

    /**
    \brief Returns (a possibly partial) list of parent folders of this folder.
    \return A vector of parents or, if this folder does not have parents,
    an empty vector. Not all parent folders may be returned by a particular call; if
    there is a large number of parent folders, several calls may be required to
    obtain all of them. An empty vector indicates end-of-iteration.
    */
    QFuture<QVector<Folder::SPtr>> parents() const;

    /**
    \brief Creates a new folder with the current folder as the parent.
    \param name The name of the new folder. Note that the actual name may be changed
    by the provider; call Item::name() once the folder is created to get its actual name.
    \return The new folder.
    */
    QFuture<Folder::SPtr> create_folder(QString const& name);

    /**
    \brief Creates a new empty file with the current folder as the parent.
    \param name The name of the new file. Note that the actual name may be changed
    by the provider; call Item::name() once the file is created to get its actual name.
    */
    QFuture<std::shared_ptr<Uploader>> create_file(QString const& name);

protected:
    Folder(internal::FolderImpl*);

    friend class FolderImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
