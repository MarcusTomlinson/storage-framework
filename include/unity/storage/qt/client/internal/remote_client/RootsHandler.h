#pragma once

#include <unity/storage/internal/ItemMetadata.h>

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

class Account;
class Root;

namespace internal
{
namespace remote_client
{

class RootsHandler : public QObject
{
    Q_OBJECT

public:
    RootsHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                 std::weak_ptr<Account> const& account);

    QFuture<QVector<std::shared_ptr<Root>>> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<QVector<std::shared_ptr<Root>>> qf_;
    std::weak_ptr<Account> account_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
