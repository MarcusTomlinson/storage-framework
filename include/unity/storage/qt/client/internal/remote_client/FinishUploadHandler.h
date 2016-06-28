#pragma once

#include <QDBusPendingReply>
#include <QFuture>
#include <QObject>

#include <memory>

namespace unity
{
namespace storage
{
namespace internal
{

class ItemMetadata;

}  // namespace internal

namespace qt
{
namespace client
{

class File;
class Root;

namespace internal
{
namespace remote_client
{

class FinishUploadHandler : public QObject
{
    Q_OBJECT

public:
    FinishUploadHandler(QDBusPendingReply<storage::internal::ItemMetadata> const& reply, std::weak_ptr<Root> root);

    QFuture<std::shared_ptr<File>> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    std::shared_ptr<Root> root_;
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<std::shared_ptr<File>> qf_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
