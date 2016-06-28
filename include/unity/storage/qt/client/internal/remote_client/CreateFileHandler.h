#pragma once

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

class Root;
class Uploader;

namespace internal
{
namespace remote_client
{

class CreateFileHandler : public QObject
{
    Q_OBJECT

public:
    CreateFileHandler(QDBusPendingReply<QString, int> const& reply,
                      std::weak_ptr<Root> const& root,
                      ProviderInterface& provider);

    QFuture<std::shared_ptr<Uploader>> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<std::shared_ptr<Uploader>> qf_;
    std::weak_ptr<Root> root_;
    ProviderInterface& provider_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
