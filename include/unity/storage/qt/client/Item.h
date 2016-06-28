#pragma once

#include <unity/storage/common.h>
#include <unity/storage/visibility.h>

#include <QDateTime>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop
#include <QString>

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Folder;
class Root;

namespace internal
{

class ItemBase;

namespace local_client
{

class UploadWorker;

}  // namespace local_client

namespace remote_client
{

class CopyHandler;
class ItemImpl;
class LookupHandler;
class MetadataHandler;

}  // namespace remote_client
}  // namespace internal

/**
\brief Base class for files and folders.
*/
class UNITY_STORAGE_EXPORT Item
{
public:
    /// @cond
    virtual ~Item();
    /// @endcond

    Item(Item&&);
    Item& operator=(Item&&);

    /**
    \brief Convenience type definition.
    */
    typedef std::shared_ptr<Item> SPtr;

    /**
    \brief Returns the native identifier used by the provider.
    */
    QString native_identity() const;

    /**
    \brief Returns the name of the file or folder.

    The returned name may not be the same as the name that was used to create the item because the provider
    may have changed it in some way (such as converting upper case characters to lower case).
    */
    QString name() const;

    /**
    \brief Returns the root folder for this item.

    If this item is a root, the returned pointer points at this item.
    */
    Root* root() const;

    /**
    \brief Returns the type of the item.
    */
    ItemType type() const;

    /**
    \brief Returns metadata for the item.

    TODO: Needs a lot more doc. Explain standard and provider-specific metadata.
    */
    QVariantMap metadata() const;

    /**
    \brief Returns the time at which the item was last modified.
    */
    QDateTime last_modified_time() const;

    /**
    \brief Returns a list of parent folders of this item.
    \return A vector of parents. For a root, the returned vector is empty.
    \warn Depending on the provider, a single file or folder may have multiple
    parents. Do not assume that only a single parent will be returned, or that
    parents are returned in a particular order.
    */
    QFuture<QVector<std::shared_ptr<Folder>>> parents() const;

    /**
    \brief Returns the native identities of the parents of this item.
    \return A vector of parent identities. For a root, the returned vector is empty.
    \warn Depending on the provider, a single file or folder may have multiple
    parents. Do not assume that only a single parent ID will be returned, or that
    parent IDs are returned in a particular order.
    */
    QVector<QString> parent_ids() const;

    /**
    \brief Copies this item.

    Copying a folder recursively copies its contents.
    \param new_parent The new parent folder for the item. If the item is to be copied within
    its current folder, this parameter must designate the currently existing parent.
    \param new_name The new name for the file.
    \warn Do not rely on copy() to fail if an attempt is made to copy
    a file or folder to a destination name that is the same as that of an already existing file or folder.
    Depending on the cloud provider, it may be possible to have several folders with the same name.
    */
    QFuture<Item::SPtr> copy(std::shared_ptr<Folder> const& new_parent, QString const& new_name);

    /**
    \brief Renames and/or moves a file or folder.
    \param new_parent The new parent folder for the item. If the item is to be renamed within
    its current folder, this parameter must designate the currently existing parent.
    \param new_name The new name for the item.
    \warn Do not rely on move() to fail if an attempt is made to move
    a file or folder to a destination name that is the same as that of an already existing file or folder.
    Depending on the cloud provider, it may be possible to have several files or folders with the same name.
    \note It is not possible to move or rename the root folder.
    */
    QFuture<Item::SPtr> move(std::shared_ptr<Folder> const& new_parent, QString const& new_name);

    /**
    \brief Permamently deletes the item.
    \warning Deleting a folder recursively deletes its contents.
    */
    QFuture<void> delete_item();

    bool equal_to(Item::SPtr const& other) const noexcept;

protected:
    Item(internal::ItemBase* p);

    std::shared_ptr<internal::ItemBase> p_;

    friend class internal::local_client::UploadWorker;
    friend class internal::remote_client::CopyHandler;
    friend class internal::remote_client::ItemImpl;
    friend class internal::remote_client::LookupHandler;
    friend class internal::remote_client::MetadataHandler;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
