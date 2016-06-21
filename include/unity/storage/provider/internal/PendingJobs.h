#pragma once

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QObject>

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

class UploadJob;

namespace internal
{


class PendingJobs : public QObject
{
    Q_OBJECT

public:
    explicit PendingJobs(QDBusConnection const& bus, QObject *parent=nullptr);
    virtual ~PendingJobs();

    void add_upload(std::unique_ptr<UploadJob> &&job);

    std::shared_ptr<UploadJob> get_upload(std::string const& upload_id);
    std::shared_ptr<UploadJob> remove_upload(std::string const& upload_id);

private Q_SLOTS:
    void service_disconnected(QString const& service_name);

private:
    void watch_peer(std::string const& bus_name);
    void unwatch_peer(std::string const& bus_name);

    std::mutex lock_;
    std::map<std::string,std::shared_ptr<UploadJob>> uploads_;

    QDBusServiceWatcher watcher_;
    std::map<std::string,int> services_;

    Q_DISABLE_COPY(PendingJobs)
};

}
}
}
}
