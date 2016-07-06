#pragma once

#include <unity/storage/internal/ItemMetadata.h>

#include <QDBusPendingReply>
#include <QFutureInterface>

#include <memory>

class ProviderInterface;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Item;
class Root;

namespace internal
{
namespace remote_client
{

class ListHandler : public QObject
{
    Q_OBJECT

public:
    ListHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>, QString> const& reply,
                std::weak_ptr<Root> const& root,
                QString const& item_id,
                ProviderInterface& provider,
                QFutureInterface<QVector<std::shared_ptr<Item>>> qf);

    QFuture<QVector<std::shared_ptr<Item>>> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    std::shared_ptr<Root> root_;
    QString item_id_;
    ProviderInterface& provider_;
    QFutureInterface<QVector<std::shared_ptr<Item>>> qf_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
