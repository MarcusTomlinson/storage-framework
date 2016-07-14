/*
 * Copyright (C) 2016 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

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
    QFuture<QVector<Item::SPtr>> lookup(QString const& name) const;

    /**
    \brief Creates a new folder with the current folder as the parent.
    \param name The name of the new folder. Note that the actual name may be changed
    by the provider; call Item::name() once the folder is created to get its actual name.
    \warn Do not rely on create_folder() to fail if an attempt is made to create
    a folder with the same name as an already existing folder or file. Depending on the cloud
    provider, it may be possible to have several folders with the same name.
    // TODO: Explain issues with metacharacters.
    \return The new folder.
    */
    QFuture<Folder::SPtr> create_folder(QString const& name);

    /**
    \brief Creates a new file with the current folder as the parent.

    Use the returned Uploader to write data to the file. You must call Uploader::finish_upload()
    for the file to actually be created (whether data was written to the file or not).
    \param name The name of the new file. Note that the actual name may be changed
    by the provider; call Item::name() once the file is created to get its actual name.
    \warn Do not rely on create_file() to fail if an attempt is made to create
    a file with the same name as an already existing file or folder. Depending on the cloud
    provider, it may be possible to have several files with the same name.
    // TODO: Explain issues with metacharacters.
    */
    QFuture<std::shared_ptr<Uploader>> create_file(QString const& name);

protected:
    Folder(internal::FolderBase*) UNITY_STORAGE_HIDDEN;

    friend class internal::local_client::FolderImpl;
    friend class internal::local_client::ItemImpl;
    friend class internal::remote_client::FolderImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
