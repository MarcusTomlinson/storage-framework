/*
 * Copyright (C) 2016 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QObject>
#pragma GCC diagnostic pop

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

class DownloadJob;
class UploadJob;

namespace internal
{


class PendingJobs : public QObject
{
    Q_OBJECT

public:
    explicit PendingJobs(QDBusConnection const& bus, QObject *parent=nullptr);
    virtual ~PendingJobs();

    void add_download(std::unique_ptr<DownloadJob> &&job);
    std::shared_ptr<DownloadJob> get_download(std::string const& download_id);
    std::shared_ptr<DownloadJob> remove_download(std::string const& download_id);

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
    std::map<std::string,std::shared_ptr<DownloadJob>> downloads_;

    QDBusServiceWatcher watcher_;
    std::map<std::string,int> services_;

    Q_DISABLE_COPY(PendingJobs)
};

}
}
}
}
