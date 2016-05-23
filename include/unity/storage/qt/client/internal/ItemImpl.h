#pragma once

#include <unity/storage/common/common.h>

#include <QDateTime>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop
#include <QString>
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

class File;
class Folder;
class Item;
class Root;

namespace internal
{

class ItemImpl : public std::enable_shared_from_this<ItemImpl>
{
public:
    ItemImpl(QString const& identity);
    ItemImpl(QString const& identity, common::ItemType type);
    virtual ~ItemImpl();
    ItemImpl(ItemImpl const&) = delete;
    ItemImpl& operator=(ItemImpl const&) = delete;

    QString native_identity() const;
    virtual QString name() const;
    unity::storage::common::ItemType type() const;
    Root* root() const;

    QFuture<QVariantMap> get_metadata() const;
    QFuture<QDateTime> last_modified_time() const;
    QFuture<std::shared_ptr<Item>> copy(std::shared_ptr<Folder> const& new_parent, QString const& new_name);
    QFuture<std::shared_ptr<Item>> move(std::shared_ptr<Folder> const& new_parent, QString const& new_name);
    virtual QFuture<QVector<std::shared_ptr<Folder>>> parents() const;
    virtual QFuture<void> destroy() = 0;

    void set_root(std::weak_ptr<Root> p);
    void set_public_instance(std::weak_ptr<Item> p);
    std::weak_ptr<Item> public_instance() const;

    static std::shared_ptr<Folder> make_folder(QString const& identity, std::weak_ptr<Root> root);
    static std::shared_ptr<File> make_file(QString const& identity, std::weak_ptr<Root> root);

protected:
    bool destroyed_ = false;
    QString identity_;
    QString name_;
    std::weak_ptr<Root> root_;
    common::ItemType type_;
    std::weak_ptr<Item> public_instance_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
