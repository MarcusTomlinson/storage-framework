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

class Downloader;
class File;

namespace internal
{
namespace remote_client
{

class DownloadHandler : public QObject
{
    Q_OBJECT

public:
    DownloadHandler(QDBusPendingReply<QString, int> const& reply,
                    std::shared_ptr<File> const& file,
                    ProviderInterface& provider);

    QFuture<std::shared_ptr<Downloader>> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    std::shared_ptr<File> file_;
    ProviderInterface& provider_;
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<std::shared_ptr<Downloader>> qf_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
