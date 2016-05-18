#pragma once

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

class Folder;
class Item;
class Root;

namespace internal
{

class ItemImpl
{
public:
    virtual ~ItemImpl();
    ItemImpl(ItemImpl const&) = delete;
    ItemImpl& operator=(ItemImpl const&) = delete;

    QString native_identity() const;
    QString name() const;
    Root* root() const;

    QFuture<QVariantMap> get_metadata() const;
    QFuture<QDateTime> last_modified_time() const;
    QFuture<QString> mime_type() const;
    QFuture<std::shared_ptr<Item>> copy(std::shared_ptr<Folder> const& new_parent, QString const& new_name);
    QFuture<std::shared_ptr<Item>> move(std::shared_ptr<Folder> const& new_parent, QString const& new_name);
    virtual QFuture<void> destroy() = 0;

    void set_root(std::weak_ptr<Root> p);
    void set_public_instance(std::weak_ptr<Item> p);
    std::weak_ptr<Item> public_instance() const;

protected:
    ItemImpl(QString const& identity);

    bool destroyed_ = false;
    QString identity_;
    QString name_;
    std::weak_ptr<Root> root_;
    std::weak_ptr<Item> public_instance_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
