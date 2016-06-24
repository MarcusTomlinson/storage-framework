#pragma once

#include <QDBusPendingReply>
#include <QFuture>
#include <QObject>

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

class UpdateHandler : public QObject
{
    Q_OBJECT

public:
    UpdateHandler(QDBusPendingReply<QString, int> const& reply,
                  QString const& old_etag,
                  std::weak_ptr<Root> root,
                  ProviderInterface& provider);

    QFuture<std::shared_ptr<Uploader>> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QString old_etag_;
    std::weak_ptr<Root> root_;
    ProviderInterface& provider_;
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<std::shared_ptr<Uploader>> qf_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
