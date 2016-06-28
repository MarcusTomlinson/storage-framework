#pragma once

#include <QDBusPendingReply>
#include <QFuture>
#include <QObject>

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{

class ItemImpl;

class DeleteHandler : public QObject
{
    Q_OBJECT

public:
    DeleteHandler(QDBusPendingReply<void> const& reply, std::shared_ptr<ItemImpl> const& impl);

    QFuture<void> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    std::shared_ptr<ItemImpl> impl_;
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<void> qf_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
