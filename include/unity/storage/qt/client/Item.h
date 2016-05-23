#pragma once

#include <unity/storage/common/common.h>

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

class ItemImpl;

}

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

    typedef std::shared_ptr<Item> SPtr;

    /**
    \brief Returns the native identifier used by the provider.
    */
    QString native_identity() const;

    /**
    \brief Returns the name of the file or directory.

    The returned name may not be the same as the name that was used to create the item because the provider
    may have changed it in some way (such as converting upper case characters to lower case).
    */
    QString name() const;

    /**
    \brief Returns the root directory for this item.

    If this item is a root, the returned pointer points at this item.
    */
    Root* root() const;

    /**
    \brief Returns metadata for the item.

    \warning The returned metadata is specific to the storage backend. Do not use this method
    for generic applications that must work with arbitrary backends.
    TODO: Needs a lot more doc. Should we provide this method at all?
    */
    QFuture<QVariantMap> metadata() const;

    /**
    \brief Returns the time at which the item was last modified.
    */
    QFuture<QDateTime> last_modified_time() const;

    /**
    \brief Returns the type of the item.
    */
    unity::storage::common::ItemType type() const;

    /**
    \brief Copies this item.

    Copying a directory recursively copies its contents.
    \param new_parent The new parent folder for the item. If the item is to be copied within
    its current directory, this parameter must designate the currently existing parent.
    \param new_name The new name for the file.
    */
    QFuture<Item::SPtr> copy(std::shared_ptr<Folder> const& new_parent, QString const& new_name);

    /**
    \brief Renames and/or moves a file or folder.
    \param new_parent The new parent folder for the item. If the item is to be renamed only,
    this parameter must designate the currently existing parent.
    \param new_name The new name for the item. If the item is to be moved to a new parent with
    the same name, this parameter must designate the currently existing name.
    \note It is not possible to move or rename the root folder.
    */
    QFuture<Item::SPtr> move(std::shared_ptr<Folder> const& new_parent, QString const& new_name);

    /**
    \brief Permamently destroys the item.
    \warning Destroying a directory recursively destroys its contents.
    */
    QFuture<void> destroy();

protected:
    Item(internal::ItemImpl* p);

    std::unique_ptr<internal::ItemImpl> p_;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
