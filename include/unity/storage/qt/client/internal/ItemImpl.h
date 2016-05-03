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

class Root;

namespace internal
{

class ItemImpl
{
public:
    ~ItemImpl();
    ItemImpl(ItemImpl const&) = delete;
    ItemImpl& operator=(ItemImpl const&) = delete;

    QString native_identity() const;
    QString name() const;
    Root* root() const;
    QVector<QString> all_names() const;
    QFuture<QVariantMap> get_metadata() const;
    QFuture<QDateTime> last_modified_time() const;
    QFuture<QString> mime_type() const;
    QFuture<void> destroy();

protected:
    ItemImpl();
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
