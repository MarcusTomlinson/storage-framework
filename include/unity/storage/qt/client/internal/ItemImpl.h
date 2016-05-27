#pragma once

#include <unity/storage/common.h>

#include <QDateTime>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop
#include <QString>
#include <QVariantMap>

#include <atomic>
#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class File;
class Folder;
class Item;
class Root;

namespace internal
{

class ItemImpl : public std::enable_shared_from_this<ItemImpl>
{
public:
    ItemImpl(QString const& identity, ItemType type);
    virtual ~ItemImpl();
    ItemImpl(ItemImpl const&) = delete;
    ItemImpl& operator=(ItemImpl const&) = delete;

    QString native_identity() const;
    virtual QString name() const;
    ItemType type() const;
    Root* root() const;
    QVariantMap metadata() const;
    QDateTime last_modified_time() const;

    QFuture<std::shared_ptr<Item>> copy(std::shared_ptr<Folder> const& new_parent, QString const& new_name);
    QFuture<std::shared_ptr<Item>> move(std::shared_ptr<Folder> const& new_parent, QString const& new_name);
    virtual QFuture<QVector<std::shared_ptr<Folder>>> parents() const;
    virtual QFuture<void> destroy();

    void set_root(std::weak_ptr<Root> p);
    void set_public_instance(std::weak_ptr<Item> p);
    std::weak_ptr<Item> public_instance() const;

    bool operator==(ItemImpl const& other) const noexcept;

protected:
    std::atomic_bool destroyed_;
    QString identity_;                     // Immutable
    QString name_;                         // Immutable
    QDateTime modified_time_;              // Immutable
    std::weak_ptr<Root> root_;             // Immutable
    QVariantMap metadata_;                 // Immutable
    ItemType type_;                        // Immutable
    std::weak_ptr<Item> public_instance_;  // Immutable
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
