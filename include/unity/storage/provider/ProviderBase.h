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
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#pragma once

#include <unity/storage/common.h>
#include <unity/storage/visibility.h>
#include <unity/storage/provider/Credentials.h>
#include <unity/storage/provider/Item.h>

#include <boost/thread/future.hpp>
#include <boost/variant.hpp>

#include <sys/types.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace unity
{
namespace storage
{
namespace provider
{

class DownloadJob;
class UploadJob;

/**
\brief Security related information for an operation invocation.

Each provider method has a trailing parameter of type Context that provides
access to
<a href="https://help.ubuntu.com/lts/serverguide/apparmor.html">Apparmor</a> details as
well as credentials for the cloud provider.
*/

struct UNITY_STORAGE_EXPORT Context
{
    uid_t uid;                   /*!< The user ID of the client process. */
    pid_t pid;                   /*!< The process ID of the client process. */
    std::string security_label;  /*!< The Apparmor security label of the client process. */

    Credentials credentials;     /*!< Credentials to authenticate with the cloud provider. */
};

/**
\brief Abstract base class for provider implementations.

The runtime calls methods on this class in response to incoming requests from clients. Each method
implements a specific operation on the storage backend.

All methods are called from the main thread and must not block. Methods indicate error conditions to the runtime
by throwing exceptions (which are caught and handled by the runtime). Alternatively, methods can return a future
that stores an exception to indicate an error.

\note Besides the explicitly listed exceptions for specific error conditions, all methods
must indicate other errors by throwing an exception dervied from StorageException, such
as PermissionException or ResourceException (see <a href="index.html#provider-error-handling">Error Handling</a>).
If an exception that does not derive from StorageException is thrown a provider method, the runtime returns a
generic UnknownException to the client.
*/

class UNITY_STORAGE_EXPORT ProviderBase : public std::enable_shared_from_this<ProviderBase>
{
public:
    ProviderBase();
    virtual ~ProviderBase();

    ProviderBase(ProviderBase const& other) = delete;
    ProviderBase& operator=(ProviderBase const& other) = delete;

    /**
    \brief Return the root folder (or folders) of the provider.
    \param keys The keys of metadata items that the client wants to receive.
    \param context The security context of the operation.
    \return A (non-empty) list of roots.
    */
    virtual boost::future<ItemList> roots(std::vector<std::string> const& keys,
                                          Context const& context) = 0;

    /**
    \brief Return a (non-recursive) list of the contents of a folder.
    \param item_id The identity of the folder.
    \param page_token A token identifying the next page of results (empty for the initial request).
    \param keys The keys of metadata items that the client wants to receive.
    \param context The security context of the operation.
    \return A tuple containing a number of child items, plus a new page token. The page token
    allows the method to return the results in multiple "pages". For the initial request, the runtime passes
    an empty <code>page_token</code>. The method returns new page token to indicate whether there are more
    results to be retrieved. If the returned token is non-empty, the runtime calls the method again,
    passing the token that was returned by the previous call. To indicate that all results were retrieved, the method
    must return an empty page token to the runtime.
    \throws InvalidArgumentException <code>item_id</code> or <code>page_token</code> are invalid.
    \throws NotExistsException <code>item_id</code> does not exist.
    \throws LogicException If at all possible, the implementation should throw a LogicException
    if <code>item_id</code> denotes a file. If a provider cannot distinguish between
    attempts to list a file and other errors, it <i>must</i> return an empty list instead.
    */
    virtual boost::future<std::tuple<ItemList,std::string>> list(std::string const& item_id,
                                                                 std::string const& page_token,
                                                                 std::vector<std::string> const& keys,
                                                                 Context const& context) = 0;

    /**
    \brief Retrieve a file or folder within a parent folder by name.
    \param parent_id The identity of the parent folder.
    \param name The name of the file or folder.
    \param keys The keys of metadata items that the client wants to receive.
    \param context The security context of the operation.
    \return A non-empty list of items with the given name. The list may contain more than one entry
    if the provider allows non-unique names for items within a folder.
    The list may also contain more than one entry with the same identity if the provider allows
    the same item to have more than one name.
    \throws InvalidArgumentException <code>parent_id</code> or <code>name</code> are invalid.
    \throws NotExistsException <code>parent_id</code> does not exist or does not contain a file or folder with
    the given <code>name</code>. (Do <i>not</i> return an empty list in this case.)
    */
    virtual boost::future<ItemList> lookup(std::string const& parent_id,
                                           std::string const& name,
                                           std::vector<std::string> const& keys,
                                           Context const& context) = 0;

    /**
    \brief Retrieve a file or folder by its identity.
    \param item_id The identity of the item.
    \param keys The keys of metadata items that the client wants to receive.
    \param context The security context of the operation.
    \return The item.
    \throws InvalidArgumentException <code>item_id</code> is invalid.
    \throws NotExistsException No file or folder with the given identity exists.
    */
    virtual boost::future<Item> metadata(std::string const& item_id,
                                         std::vector<std::string> const& keys,
                                         Context const& context) = 0;

    /**
    \brief Create a new folder.
    \param parent_id The identity of the parent folder.
    \param name The name of the new folder.
    \param keys The keys of metadata items that the client wants to receive.
    \param context The security context of the operation.
    \return The item representing the folder.
    \throws InvalidArgumentException <code>parent_id</code> or <code>name</code> are invalid.
    \throws ExistsException An item with the given <code>name</code> exists already.
    */
    virtual boost::future<Item> create_folder(std::string const& parent_id,
                                              std::string const& name,
                                              std::vector<std::string> const& keys,
                                              Context const& context) = 0;

    /**
    \brief Create a new file.
    \param parent_id The identity of the parent folder.
    \param name The name of the new file.
    \param size The size of the file contents in bytes.
    \param content_type The mime type of the file. If empty, the provider may (or may not) determine the mime type automatically.
    \param allow_overwrite If true, the file will be created even if a file with the same name exists already.
    \param keys The keys of metadata items that the client wants to receive.
    \param context The security context of the operation.
    \return An UploadJob that will read any data provided by the client and write it to the new file.
    \throws InvalidArgumentException <code>parent_id</code> or <code>name</code> are invalid.
    \throws ExistsException A file with the given <code>name</code> exists already and
    <code>allow_overwrite</code> is <code>false</code>, or a folder with the given <code>name</code> exists already.
    */
    // TODO: The runtime should check that size is non-negative, so the provider implementation can rely on this.
    virtual boost::future<std::unique_ptr<UploadJob>> create_file(std::string const& parent_id,
                                                                  std::string const& name,
                                                                  int64_t size,
                                                                  std::string const& content_type,
                                                                  bool allow_overwrite,
                                                                  std::vector<std::string> const& keys,
                                                                  Context const& context) = 0;

    /**
    \brief Update the contents of a file.
    \param item_id The identity of the file.
    \param size The size of the file contents in bytes.
    \param old_etag The ETag of the existing file (empty if the file should be overwritten).
    \param keys The keys of metadata items that the client wants to receive.
    \param context The security context of the operation.
    \return An UploadJob that will read any data provided by the client and write it to the file.
    \throws InvalidArgumentException <code>item_id</code> or <code>size</code> are invalid.
    \throws ConflictException The file's ETag does not match the given (non-empty) <code>old_etag</code>.
    \throws LogicException The <code>item_id</code> denotes a folder.
    */
    // TODO: The runtime should check that size is non-negative, so the provider implementation can rely on this.
    virtual boost::future<std::unique_ptr<UploadJob>> update(std::string const& item_id,
                                                             int64_t size,
                                                             std::string const& old_etag,
                                                             std::vector<std::string> const& keys,
                                                             Context const& context) = 0;

    /**
    \brief Download the contents of a file.
    \param item_id The identity of the file.
    \param match_etag The ETag of the existing file (empty if the file should be downloaded unconditionally).
    \param context The security context of the operation.
    \return A DownloadJob that will write the file data to the client socket.
    \throws InvalidArgumentException <code>item_id</code> or <code>name</code> are invalid.
    \throws NotExistsException <code>item_id</code> does not exist.
    \throws LogicException The <code>item_id</code> denotes a folder.
    \throws ConflictException The ETag for <code>item_id</code> does not match the given
    (non-empty) <code>match_etag</code>.
    */
    virtual boost::future<std::unique_ptr<DownloadJob>> download(std::string const& item_id,
                                                                 std::string const& match_etag,
                                                                 Context const& context) = 0;

    /**
    \brief Delete an item.

    Deletion is recursive. If <code>item_id</code> denotes a folder, the folder and all its contents are deleted.
    \param item_id The identity of the file or folder.
    \param context The security context of the operation.
    \throws InvalidArgumentException <code>item_id</code> is invalid.
    \throws PermissionException <code>item_id</code> denotes a root (or permission was denied for another reason).
    */
    virtual boost::future<void> delete_item(std::string const& item_id,
                                            Context const& context) = 0;

    /**
    \brief Move and/or rename an item.

    \param item_id The identity of the file or folder to be moved.
    \param new_parent_id The identity of the folder to move the file or folder to. If <code>new_parent_id</code>
    is the same as the existing parent of <code>item_id</code>, the file or folder is to be renamed within its
    parent folder.
    \param new_name The new name of the file (which can be the same as the old name if <code>new_parent_id</code>
    differs).
    \param keys The keys of metadata items that the client wants to receive.
    \param context The security context of the operation.
    \return The moved or renamed item.
    \throws InvalidArgumentException <code>item_id</code>, <code>new_parent_id</code>, or <code>new_name</code>
    are invalid.
    \throws ExistsException The folder <code>new_parent_id</code> already contains an item with
    name <code>new_name</code>.
    \throws PermissionException <code>item_id</code> denotes a root (or permission was denied for another reason).
    */
    virtual boost::future<Item> move(std::string const& item_id,
                                     std::string const& new_parent_id,
                                     std::string const& new_name,
                                     std::vector<std::string> const& keys,
                                     Context const& context) = 0;

    /**
    \brief Copy an item.

    Copy is recursive, so copying a folder copies the folder's contents.
    \param item_id The identity of the file or folder to be copied.
    \param new_parent_id The identity of the folder to copy the file or folder to. If <code>new_parent_id</code>
    is the same as the existing parent of <code>item_id</code>, the file or folder is to be copied within its
    parent folder.
    \param new_name The new name of the file or folder.
    \param keys The keys of metadata items that the client wants to receive.
    \param context The security context of the operation.
    \return The copied item.
    \throws InvalidArgumentException <code>item_id</code>, <code>new_parent_id</code>, or <code>new_name</code>
    are invalid.
    \throws ExistsException The folder <code>new_parent_id</code> already contains an item with
    name <code>new_name</code>.
    */
    virtual boost::future<Item> copy(std::string const& item_id,
                                     std::string const& new_parent_id,
                                     std::string const& new_name,
                                     std::vector<std::string> const& keys,
                                     Context const& context) = 0;
};

}
}
}
