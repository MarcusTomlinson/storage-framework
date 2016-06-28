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

class Folder;
class Root;

namespace internal
{
namespace remote_client
{

class CreateFolderHandler : public QObject
{
    Q_OBJECT

public:
    CreateFolderHandler(QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                        std::weak_ptr<Root> const& root);

    QFuture<std::shared_ptr<Folder>> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<std::shared_ptr<Folder>> qf_;
    std::weak_ptr<Root> root_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
