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

class CancelUploadHandler : public QObject
{
    Q_OBJECT

public:
    CancelUploadHandler(QDBusPendingReply<void> const& reply);

    QFuture<void> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<void> qf_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
