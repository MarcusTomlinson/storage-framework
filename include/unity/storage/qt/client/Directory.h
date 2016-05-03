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

class DirectoryImpl;

}  // namespace internal

/**
\brief Class that represents a directory.

A directory is an unordered set of files and/or directories. The names (as returned by Item::name())
of the members of a directory are unique.
*/
class Directory : public Item
{
public:
    ~Directory();
    Directory(Directory const&) = delete;
    Directory& operator=(Directory const&) = delete;
    Directory(Directory&&);
    Directory& operator=(Directory&&);

    typedef std::unique_ptr<Directory> UPtr;

    /**
    \brief Returns (possibly partial) contents of a directory.
    \return A vector of items or, if this directory is empty,
    an empty vector. Not all items may be returned by a particular call; if
    there is a large number of items, several calls may be required to
    obtain all of them. An empty vector indicates end-of-iteration.
    */
    QFuture<QVector<Item::UPtr>> list() const;

    /**
    \brief Returns (a possibly partial) list of parent directories of this directory.
    \return A vector of parents or, if this directory does not have parents,
    an empty vector. Not all parent directories may be returned by a particular call; if
    there is a large number of parent directories, several calls may be required to
    obtain all of them. An empty vector indicates end-of-iteration.
    */
    QFuture<QVector<Directory::UPtr>> parents() const;

    /**
    \brief Creates a new directory with the current directory as the parent.
    \param name The name of the new directory. Note that the actual name may be changed
    by the provider; call Item::name() once the directory is created to get its actual name.
    \return The new directory.
    */
    QFuture<Directory::UPtr> create_dir(QString const& name);

    /**
    \brief Creates a new empty file with the current directory as the parent.
    \param name The name of the new file. Note that the actual name may be changed
    by the provider; call Item::name() once the file is created to get its actual name.
    */
    QFuture<File::UPtr> create_file(QString const& name);

protected:
    Directory(internal::DirectoryImpl*);

    friend class DirectoryImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
