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

#include <unity/storage/common.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDateTime>
#include <QVariantMap>
#pragma GCC diagnostic pop

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class ItemImpl;
class DownloaderImpl;
class UploaderImpl;

}  // namespace internal

class Account;
class Downloader;
class IntJob;
class ItemJob;
class ItemListJob;
class Uploader;
class VoidJob;

/**
Class that provides access to a file or folder.

Item provides access to the details of a file or folder, such as its identity, name, etc.
It also provides operations to, for example, create or destroy files or folders, and to
initiate uploads and downloads.

Note that this class is an immutable value type: if you retrieve the details of an item, and
those details change due to other operations being performed on the item, any changes are <i>not</i> reflected
by the Item instance you retrieved earlier. Each operation that potentially modifies the state of an item
also returns the new update item once it completes, so you can assign the update item to the original item or,
alternatively, simply let the original item go out scope after initiating an operation.
*/

class Q_DECL_EXPORT Item final
{
    Q_GADGET

    /**
    \see \link itemId() const itemId()\endlink
    */
    Q_PROPERTY(QString itemId READ itemId FINAL)

    /**
    \see \link name() const name()\endlink
    */
    Q_PROPERTY(QString name READ name FINAL)

    /**
    \see \link account() const account()\endlink
    */
    Q_PROPERTY(unity::storage::qt::Account account READ account FINAL)

    /**
    \see \link etag() const etag()\endlink
    */
    Q_PROPERTY(QString etag READ etag FINAL)

    /**
    \see \link type() const type()\endlink
    */
    Q_PROPERTY(unity::storage::qt::Item::Type type READ type FINAL)

    /**
    \see \link metadata() const metadata()\endlink
    */
    Q_PROPERTY(QVariantMap metadata READ metadata FINAL)

    /**
    \see \link lastModifiedTime() const lastModifiedTime()\endlink
    */
    Q_PROPERTY(QDateTime lastModifiedTime READ lastModifiedTime FINAL)

    /**
    \see \link parentIds() const parentIds()\endlink
    */
    Q_PROPERTY(QStringList parentIds READ parentIds FINAL)

public:
    /**
    \brief Constructs an item.

    A default-constructed Item returns <code>false</code> from isValid(), and the remaining accessors return
    default-constructed values.
    */
    Item();

    /**
    \brief Destroys an item.
    */
    ~Item();

    /** @name Copy and assignment
    \brief Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    Item(Item const&);
    Item(Item&&);
    Item& operator=(Item const&);
    Item& operator=(Item&&);
    //@}

    /**
    \brief Indicates the type of the item.
    */
    enum Type
    {
        File /** @cond */ = unsigned(unity::storage::ItemType::file) /** @endcond */,     /*!< The item is a file. */
        Folder /** @cond */ = unsigned(unity::storage::ItemType::folder) /** @endcond */, /*!< The item is a folder. */
        Root /** @cond */ = unsigned(unity::storage::ItemType::root) /** @endcond */      /*!< The item is a root folder. */
    };
    Q_ENUMS(Type)

    /**
    \brief Indicates how the provider should respond in case of an ETag mismatch.
    */
    enum ConflictPolicy
    {
        ErrorIfConflict /** @cond */
            = unsigned(unity::storage::ConflictPolicy::error_if_conflict) /** @endcond */, /*!< Fail the operation. */
        IgnoreConflict /** @cond */
            = unsigned(unity::storage::ConflictPolicy::ignore_conflict)   /** @endcond */  /*!< Ignore the ETag mismatch. */
    };
    Q_ENUMS(ConflictPolicy)

    /** @name Accessors
    */
    //{@

    /**
    \brief Checks whether this item was successfully constructed.
    \return Returns <code>true</code> if the item contains valid details; <code>false</code> otherwise.
    */
    bool isValid() const;

    /**
    \brief Returns the identity of the item.

    Identities are unique only within their corresponding account, so items belong to different accounts may
    have the same identity. If an item is destroyed, it is possible for a different item that is created later
    to get the destroyed item's identity.
    \return The identity of the item.
    \see \link identity File and Folder Identity\endlink
    */
    QString itemId() const;

    /**
    \brief Returns the name of the item.

    Providers may silently modify the name of files and folders that are created. (For example, a provider
    may map upper-case letters to lower-case.) After creating an item,
    always call name() to obtain the actual name of the item. 
    \return The item's name.
    \see \link names File and Folder Names\endlink
    */
    QString name() const;

    /**
    \brief Returns the account for the item.
    \return The item's account.
    \see \link accounts Accounts\endlink
    */
    Account account() const;

    /**
    \brief Returns the ETag of the item.
    \return The item's ETag.
    \see \link etags ETags\endlink
    */
    QString etag() const;

    /**
    \brief Returns the type of the item.
    \return The item's type
    */
    Type type() const;

    /**
    \brief Returns the metadata for the item.
    \return The item's metadata
    \see \link metadata Metadata\endlink
    */
    QVariantMap metadata() const;

    /**
    \brief Returns the size in bytes of a file.
    \return The file size. If the item is not a file, the return value is 0.
    */
    qint64 sizeInBytes() const;

    /**
    \brief Returns the time at which the item was last modified.
    \return The item's modification time. For files, the modification time is always available.
    Some providers do not support modification time stamps for folders. If so, the returned
    time is invalid.
    */
    QDateTime lastModifiedTime() const;

    /**
    \brief Returns the identity of the item's parent folders.
    \return The item's parent identities.
    */
    QStringList parentIds() const;
    //@}

    /** @name Operations
    All operations are asynchronous and return a job that, once complete, provides the return value
    or error information.
    \note You <i>must</i> deallocate the returned instance by calling <code>delete</code> (or, alternatively,
    calling <a href="http://doc.qt.io/qt-5/qobject.html#setParent">setParent()</a> to re-parent the job).
    Failing to do so causes a memory leak!
    */
    //{@
    /**
    \brief Creates a job that retrieves the parent folders of this item.
    \return A job that, once complete, provides access to the parent folders.
    */
    Q_INVOKABLE unity::storage::qt::ItemListJob* parents(QStringList const& keys = QStringList()) const;

    /**
    \brief Copies this file or folder.

    Copying is recursive. If you copy a folder, all it's contents are copied.

    You can copy files and folders only within the same account.
    \param newParent The parent folder for this item. If you want to copy this item within
    the same parent folder, <code>newParent</code> must be the same as this item's current parent.
    \param newName The new name for this item within its <code>newParent</code> folder.
    \param keys A list of metadata keys for metadata items that should be returned by the provider.
    If the list is empty, the provider returns a default set of metadata items.
    \return A job that, once complete, provides access to the copied item.
    */
    Q_INVOKABLE unity::storage::qt::ItemJob* copy(Item const& newParent,
                              QString const& newName,
                              QStringList const& keys = QStringList()) const;

    /**
    \brief Moves and/or rename this file or folder.

    You can move files and folders only within the same account.
    \param newParent The parent folder for this item. If you want to rename this item within
    the same parent folder, <code>newParent</code> must be the same as this item's current parent.
    \param newName The new name for this item within its <code>newParent</code> folder.
    \param keys A list of metadata keys for metadata items that should be returned by the provider.
    If the list is empty, the provider returns a default set of metadata items.
    \return A job that, once complete, provides access to the copied item.
    */
    Q_INVOKABLE unity::storage::qt::ItemJob* move(Item const& newParent,
                              QString const& newName,
                              QStringList const& keys = QStringList()) const;

    /**
    \brief Deletes this item.

    Deletion is recursive. If this item is a folder, all it's contents are deleted as well.

    You cannot delete a root folder.
    \return A job that, once complete, provides access to a StorageError (if any).
    */
    Q_INVOKABLE unity::storage::qt::VoidJob* deleteItem() const;

    /**
    \brief Creates an uploader for this file.

    Attempts to upload to a folder return an uploader that indicates an error.
    \param policy If set to <code>ErrorIfConflict</code>, the upload job indicates an error if this file's
    ETag no longer matches the ETag maintained by the provider. If set to <code>IgnoreConflict</code>, this
    file's contents are overwritten.
    \param sizeInBytes The size of the upload. You must write <i>exactly</i> that number of bytes to the upload
    socket before finalizing the upload. If the <code>sizeInBytes</code> does not match the size of the
    actually uploaded data, the upload will fail.
    \param keys A list of metadata keys for metadata items that should be returned by the provider.
    If the list is empty, the provider returns a default set of metadata items.
    \return An uploader that, once ready, can be used to upload the data for this file.
    \see \link uploads-downloads Uploads and Downloads\endlink
    */
    Q_INVOKABLE unity::storage::qt::Uploader* createUploader(ConflictPolicy policy,
                                                             qint64 sizeInBytes,
                                                             QStringList const& keys = QStringList()) const;

    /**
    \brief Creates a downloader for this file.

    Attempts to download a folder return a downloader that indicates an error.
    \param policy If set to <code>ErrorIfConflict</code>, the download job indicates an error if this file's
    ETag no longer matches the ETag maintained by the provider. If set to <code>IgnoreConflict</code>, the
    download will proceed regardless of any ETag mismatch.
    \return A downloader that, once ready, can be used to download the data for this file.
    \see \link uploads-downloads Uploads and Downloads\endlink
    */
    Q_INVOKABLE unity::storage::qt::Downloader* createDownloader(ConflictPolicy policy) const;

    /**
    \brief Lists the contents of this folder.

    Attempts to list a file return a job that indicates an error.
    \param keys A list of metadata keys for metadata items that should be returned by the provider.
    If the list is empty, the provider returns a default set of metadata items.
    \return A job that, once complete, provides access to this folder's items.
    */
    Q_INVOKABLE unity::storage::qt::ItemListJob* list(QStringList const& keys = QStringList()) const;

    /**
    \brief Locates an item by name within this folder.

    Attempts to perform a lookup on a file return a job that indicates an error.
    \param name The name of the item to look up.
    \param keys A list of metadata keys for metadata items that should be returned by the provider.
    If the list is empty, the provider returns a default set of metadata items.
    \return A job that, once complete, provides access to the items with the given <code>name</code>.
    */
    Q_INVOKABLE unity::storage::qt::ItemListJob* lookup(QString const& name,
                                                        QStringList const& keys = QStringList()) const;

    /**
    \brief Creates a child folder within this folder.

    Attempts to create a folder within a file return a job that indicates an error.
    \param name The name of the new folder within this folder.
    \param keys A list of metadata keys for metadata items that should be returned by the provider.
    If the list is empty, the provider returns a default set of metadata items.
    */
    Q_INVOKABLE unity::storage::qt::ItemJob* createFolder(QString const& name,
                                                          QStringList const& keys = QStringList()) const;

    /**
    \brief Creates a file within this folder.

    The number of bytes written via the returned uploader <i>must</i> match <code>sizeInBytes</code>, otherwise
    the upload will indicate an error.

    Attempts to create a file within a file return a job that indicates an error.
    \param name The name of the new file within this folder.
    \param policy If set to <code>ErrorIfConflict</code>, the upload job indicates an error if this file's
    ETag no longer matches the ETag maintained by the provider. If set to <code>IgnoreConflict</code>, any
    existing file with the same name is overwritten.
    \param sizeInBytes The number of bytes that the client will upload for this file.
    \param contentType The <a href="https://www.iana.org/assignments/media-types/media-types.xhtml">media type</a>
    of the file. Note that providers may ignore this parameter and determine the media type
    themselves (based on the file name or the file contents).
    \param keys A list of metadata keys for metadata items that should be returned by the provider.
    If the list is empty, the provider returns a default set of metadata items.
    */
    Q_INVOKABLE unity::storage::qt::Uploader* createFile(QString const& name,
                                                         ConflictPolicy policy,
                                                         qint64 sizeInBytes,
                                                         QString const& contentType,
                                                         QStringList const& keys = QStringList()) const;
    //@}

    /** @name Comparison operators and hashing.
    */
    //{@
    /**
    \brief Compares Item instances for equality.

    Item instances are equal if both are invalid, or both are valid, use the same account,
    and have the same identity.
    \param other The item to compare this item with.
    \return If all details of the items match, <code>true</code> is returned; <code>false</code> otherwise.
    */
    bool operator==(Item const& other) const;
    bool operator!=(Item const&) const;
    bool operator<(Item const&) const;
    bool operator<=(Item const&) const;
    bool operator>(Item const&) const;
    bool operator>=(Item const&) const;

    /**
    \brief Returns a hash value.
    \note The hash value is <i>not</i> necessarily the same as the one returned by
    \link unity::storage::qt::qHash(Item const& i) qHash()\endlink, but <i>is</i> the same as the one returned by
    \link std::hash<unity::storage::qt::Item> Item::hash()\endlink.
    \return A hash value for use with unordered containers.
    */
    size_t hash() const;
    //@}

private:
    ///@cond
    Item(std::shared_ptr<internal::ItemImpl> const&);

    std::shared_ptr<internal::ItemImpl> p_;

    friend class internal::ItemImpl;
    friend class internal::DownloaderImpl;
    friend class internal::UploaderImpl;
    ///@endcond
};

/**
\brief Returns a hash value.
\note The hash value is <i>not</i> necessarily the same as the one returned by
\link Item::hash()\endlink.
\return A hash value for use with unordered containers.
*/
uint Q_DECL_EXPORT qHash(unity::storage::qt::Item const& i);

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::Item)
Q_DECLARE_METATYPE(QList<unity::storage::qt::Item>)
Q_DECLARE_METATYPE(unity::storage::qt::Item::Type)
Q_DECLARE_METATYPE(unity::storage::qt::Item::ConflictPolicy)

namespace std
{

/**
\brief Function template specialization in namespace <code>std</code> to allow use of
\link unity::storage::qt::Item Item\endlink instances with unordered containers.
*/
template<> struct Q_DECL_EXPORT hash<unity::storage::qt::Item>
{
    /**
    \brief Returns a hash value.
    \note The hash value is <i>not</i> necessarily the same as the one returned by
    \link unity::storage::qt::qHash(const Item& i) qHash()\endlink (but <i>is</i> the same as the value returned
    by \link unity::storage::qt::Item::hash() Item::hash()\endlink).
    \return A hash value for use with unordered containers.
    \see \link unity::storage::qt::Item::hash Item::hash()\endlink
    */
    std::size_t operator()(unity::storage::qt::Item const& i) const
    {
        return i.hash();
    }
};

}  // namespace std
