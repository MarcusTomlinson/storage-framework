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

class FileBase;

namespace local_client
{

class FileImpl;

}  // namespace local_client

namespace remote_client
{

class FileImpl;

}  // namespace remotelocal_client
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
    \param policy The conflict resolution policy. If set to ConflictPolicy::overwrite,
    the contents of the file will be overwritten even if the file was modified
    after this File instance was retrieved. Otherwise, if set to ConflictPolicy::error_if_conflict,
    an attempt to retrieve the File instance from the future returned by Uploader::finish_upload()
    throws ConflictException if the file was was modified via some other channel.
    \param size The size of the upload in bytes.
    \note The provided file size must match the number of bytes that you write for the upload, otherwise
    an attampt to retrive the File instance from the future returned by Uploader::finish_upload()
    throws LogicException.
    */
    QFuture<std::shared_ptr<Uploader>> create_uploader(ConflictPolicy policy, int64_t size);

    /**
    \brief Creates a downloader for the file.
    */
    QFuture<std::shared_ptr<Downloader>> create_downloader();

private:
    File(internal::FileBase*) UNITY_STORAGE_HIDDEN;

    friend class internal::local_client::FileImpl;
    friend class internal::remote_client::FileImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
