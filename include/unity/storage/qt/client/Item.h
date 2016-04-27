#pragma once

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

class Root;

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
    QString native_identity() const;

    /**
    \brief Returns the name of the file or directory.

    The returned name may not be the same as the name that was used to create the item because the provider
    may have changed it in some way (such as converting upper case characters to lower case).
    */
    QString name() const;

    /**
    \brief Returns all names for this item.

    Depending on the storage provider, an item may have more than one name, and may even have the same name
    within the same directory more than once.
    \return Returns all known names for this item within its parent directory, in no particular order.
    */
    QVector<QString> all_names() const;

    /**
    \brief Returns metadata for the item.

    \warning The returned metadata is specific to the storage backend. Do not use this method
    for generic applications that must work with arbitrary backends.
    TODO: Needs a lot more doc. Should we provide this method at all?
    */
    QFuture<QVariantMap> get_metadata() const;

    /**
    \brief Returns the time at which the item was last modified.
    */
    QFuture<QDateTime> last_modified_time() const;

    /**
    \brief Returns the mime type of the item.
    \return For directories, the mime type is `inode/directory`. If the mime type is unknown,
    the returned string is empty.
    */
    QFuture<QString> mime_type() const;

    /**
    \brief Returns the root directory for this item.

    If this item is a root, the returned pointer points at this item.
    */
    Root* root() const;

    /**
    \brief Permamently destroys the item.
    \warning Destroying a directory recursively destroys its contents.
    */
    QFuture<void> destroy();

protected:
    Item();
};

}  // namespace client

}  // namespace qt

}  // namespace storage

}  // namespace unity
