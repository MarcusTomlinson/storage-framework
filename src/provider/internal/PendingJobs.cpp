#include <unity/storage/provider/internal/PendingJobs.h>
#include <unity/storage/provider/DownloadJob.h>
#include <unity/storage/provider/UploadJob.h>
#include <unity/storage/provider/internal/DownloadJobImpl.h>
#include <unity/storage/provider/internal/MainLoopExecutor.h>
#include <unity/storage/provider/internal/UploadJobImpl.h>

#include <cassert>
#include <cstdio>
#include <stdexcept>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

PendingJobs::PendingJobs(QDBusConnection const& bus, QObject *parent)
    : QObject(parent)
{
    watcher_.setConnection(bus);
    watcher_.setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(&watcher_, &QDBusServiceWatcher::serviceUnregistered,
            this, &PendingJobs::service_disconnected);
}

PendingJobs::~PendingJobs() = default;

void PendingJobs::add_download(unique_ptr<DownloadJob> &&job)
{
    lock_guard<mutex> guard(lock_);

    assert(!job->p_->sender_bus_name().empty());
    assert(downloads_.find(job->download_id()) == downloads_.end());

    shared_ptr<DownloadJob> j(std::move(job));
    downloads_.emplace(j->download_id(), j);
    watch_peer(j->p_->sender_bus_name());
}

std::shared_ptr<DownloadJob> PendingJobs::get_download(std::string const& download_id)
{
    lock_guard<mutex> guard(lock_);

    return downloads_.at(download_id);
}

std::shared_ptr<DownloadJob> PendingJobs::remove_download(std::string const& download_id)
{
    lock_guard<mutex> guard(lock_);

    auto job = downloads_.at(download_id);
    downloads_.erase(download_id);
    unwatch_peer(job->p_->sender_bus_name());
    return job;
}

void PendingJobs::add_upload(unique_ptr<UploadJob> &&job)
{
    lock_guard<mutex> guard(lock_);

    assert(!job->p_->sender_bus_name().empty());
    assert(uploads_.find(job->upload_id()) == uploads_.end());

    shared_ptr<UploadJob> j(std::move(job));
    uploads_.emplace(j->upload_id(), j);
    watch_peer(j->p_->sender_bus_name());
}

std::shared_ptr<UploadJob> PendingJobs::get_upload(std::string const& upload_id)
{
    lock_guard<mutex> guard(lock_);

    return uploads_.at(upload_id);
}

std::shared_ptr<UploadJob> PendingJobs::remove_upload(std::string const& upload_id)
{
    lock_guard<mutex> guard(lock_);

    auto job = uploads_.at(upload_id);
    uploads_.erase(upload_id);
    unwatch_peer(job->p_->sender_bus_name());
    return job;
}

void PendingJobs::watch_peer(string const& bus_name)
{
    auto it = services_.find(bus_name);
    if (it != services_.end())
    {
        it->second++;
    }
    else
    {
        watcher_.addWatchedService(QString::fromStdString(bus_name));
        services_[bus_name] = 1;
    }
}

void PendingJobs::unwatch_peer(string const& bus_name)
{
    auto it = services_.find(bus_name);
    if (it == services_.end())
    {
        return;
    }
    it->second--;
    if (it->second == 0)
    {
        services_.erase(it);
        watcher_.removeWatchedService(QString::fromStdString(bus_name));
    }
}

void PendingJobs::service_disconnected(QString const& service_name)
{
    lock_guard<mutex> guard(lock_);
    string const bus_name = service_name.toStdString();

    for (auto it = downloads_.cbegin(); it != downloads_.cend(); )
    {
        if (it->second->p_->sender_bus_name() == bus_name)
        {
            auto job = it->second;
            it = downloads_.erase(it);
            auto f = job->p_->cancel(*job);
            // This continuation also ensures that the job remains
            // alive until the cancel method has completed.
            f.then(
                [job](decltype(f) f) {
                    try
                    {
                        f.get();
                    }
                    catch (std::exception const& e)
                    {
                        fprintf(stderr, "Error cancelling download job '%s': %s\n",
                                job->download_id().c_str(), e.what());
                    }
                });
        }
        else
        {
            ++it;
        }
    }

    for (auto it = uploads_.cbegin(); it != uploads_.cend(); )
    {
        if (it->second->p_->sender_bus_name() == bus_name)
        {
            auto job = it->second;
            it = uploads_.erase(it);
            auto f = job->cancel();
            // This continuation also ensures that the job remains
            // alive until the cancel method has completed.
            f.then(
                [job](decltype(f) f) {
                    try
                    {
                        f.get();
                    }
                    catch (std::exception const& e)
                    {
                        fprintf(stderr, "Error cancelling upload job '%s': %s\n",
                                job->upload_id().c_str(), e.what());
                    }
                });
        }
        else
        {
            ++it;
        }
    }
}

}
}
}
}
