#pragma once

#include <unity/storage/common.h>
#include <unity/storage/qt/client/internal/boost_filesystem.h>

#include <QDateTime>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop
#include <QVariantMap>

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
class Item;
class Root;

typedef QMap<QString, QVariant> MetadataMap;

namespace internal
{

class ItemImpl;

class ItemBase : public std::enable_shared_from_this<ItemBase>
{
public:
    ItemBase(QString const& identity, ItemType type);
    virtual ~ItemBase();
    ItemBase(ItemBase const&) = delete;
    ItemBase& operator=(ItemBase const&) = delete;

    QString native_identity() const;
    ItemType type() const;
    Root* root() const;

    virtual QString name() const = 0;
    virtual QDateTime last_modified_time() const = 0;

    virtual QFuture<std::shared_ptr<Item>> copy(std::shared_ptr<Folder> const& new_parent, QString const& new_name) = 0;
    virtual QFuture<std::shared_ptr<Item>> move(std::shared_ptr<Folder> const& new_parent, QString const& new_name) = 0;
    virtual QFuture<QVector<std::shared_ptr<Folder>>> parents() const = 0;
    virtual QVector<QString> parent_ids() const = 0;
    virtual QFuture<void> delete_item() = 0;

    virtual QDateTime creation_time() const = 0;
    virtual MetadataMap native_metadata() const = 0;

    virtual bool equal_to(ItemBase const& other) const noexcept = 0;

    void set_root(std::weak_ptr<Root> p);
    void set_public_instance(std::weak_ptr<Item> p);

protected:
    const QString identity_;
    const ItemType type_;
    std::weak_ptr<Root> root_;
    std::weak_ptr<Item> public_instance_;

    friend class ItemImpl;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
