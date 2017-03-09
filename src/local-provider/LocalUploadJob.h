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

#include <unity/storage/provider/UploadJob.h>

#include <unity/util/ResourcePtr.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <QFile>
#include <QLocalSocket>
#pragma GCC diagnostic pop

class LocalProvider;

class LocalUploadJob : public QObject, public unity::storage::provider::UploadJob
{
    Q_OBJECT
public:
    LocalUploadJob(std::shared_ptr<LocalProvider> const& provider, int64_t size, const std::string& method);

    // create_file()
    LocalUploadJob(std::shared_ptr<LocalProvider> const& provider,
                   std::string const& parent_id,
                   std::string const& name,
                   int64_t size,
                   bool allow_overwrite);
    // update()
    LocalUploadJob(std::shared_ptr<LocalProvider> const& provider,
                   std::string const& item_id,
                   int64_t size,
                   std::string const& old_etag);
    virtual ~LocalUploadJob() = default;

    virtual boost::future<void> cancel() override;
    virtual boost::future<unity::storage::provider::Item> finish() override;

private Q_SLOTS:
    void on_bytes_ready();
    void on_read_channel_finished();

private:
    enum State { in_progress, finished, cancelled };

    void read_and_write_chunk();
    void abort_upload();

    std::shared_ptr<LocalProvider> const provider_;
    int64_t const size_;
    int64_t bytes_to_write_;
    std::unique_ptr<QFile> file_;
    QLocalSocket read_socket_;
    std::string const method_;
    State state_;
    std::string item_id_;
    std::string old_etag_;   // Empty for create_file()
    std::string parent_id_;  // Empty for update()
    bool allow_overwrite_;   // Undefined for update()
    unity::util::ResourcePtr<int, std::function<void(int)>> tmp_fd_;
    bool use_linkat_;
};
