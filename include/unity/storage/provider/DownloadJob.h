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

#include <unity/storage/visibility.h>

#include <boost/thread/future.hpp>
#include <stdexcept>
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{
class DownloadJobImpl;
class PendingJobs;
class ProviderInterface;
}

class UNITY_STORAGE_EXPORT DownloadJob
{
public:
    DownloadJob(std::string const& download_id);
    virtual ~DownloadJob();

    std::string const& download_id() const;
    int write_socket() const;

    // If the result of the download is reported with either of the
    // following two functions, then neither cancel() or finish() will
    // be called.
    void report_complete();
    void report_error(std::exception_ptr p);

    virtual boost::future<void> cancel() = 0;
    virtual boost::future<void> finish() = 0;

protected:
    DownloadJob(internal::DownloadJobImpl *p) UNITY_STORAGE_HIDDEN;
    internal::DownloadJobImpl *p_ = nullptr;

    friend class internal::PendingJobs;
    friend class internal::ProviderInterface;
};

}
}
}
