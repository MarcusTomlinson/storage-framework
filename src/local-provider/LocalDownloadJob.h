/*
 * Copyright (C) 2017 Canonical Ltd
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
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <unity/storage/provider/DownloadJob.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <QFile>
#include <QLocalSocket>
#pragma GCC diagnostic pop

class LocalProvider;

class LocalDownloadJob : public QObject, public unity::storage::provider::DownloadJob
{
    Q_OBJECT
public:
    LocalDownloadJob(std::shared_ptr<LocalProvider> const& provider,
                     std::string const& item_id,
                     std::string const& match_etag);
    virtual ~LocalDownloadJob();

    virtual boost::future<void> cancel() override;
    virtual boost::future<void> finish() override;

private Q_SLOTS:
    void on_bytes_written(qint64 bytes);
    void read_and_write_chunk();

private:
    std::shared_ptr<LocalProvider> const provider_;
    std::string const item_id_;
    std::unique_ptr<QFile> file_;
    QLocalSocket write_socket_;
    int64_t bytes_to_write_;
};
