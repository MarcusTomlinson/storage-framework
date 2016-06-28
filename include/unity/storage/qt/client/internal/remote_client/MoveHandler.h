#pragma once

#include <unity/storage/internal/ItemMetadata.h>

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFutureInterface>

#include <memory>

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

class MoveHandler : public QObject
{
    Q_OBJECT

public:
    MoveHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                std::weak_ptr<Root> const& root);

    QFuture<std::shared_ptr<Item>> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<std::shared_ptr<Item>> qf_;
    std::shared_ptr<Root> root_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
