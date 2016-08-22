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
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

struct Item;

namespace internal
{
class PendingJobs;
class ProviderInterface;
class UploadJobImpl;
}

class UNITY_STORAGE_EXPORT UploadJob
{
public:
    UploadJob(std::string const& upload_id);
    virtual ~UploadJob();

    std::string const& upload_id() const;
    int read_socket() const;

    // If an error is reported early, cancel() or finish() will not be
    // invoked.
    void report_error(std::exception_ptr p);

    virtual boost::future<void> cancel() = 0;
    virtual boost::future<Item> finish() = 0;

    // Called when finishing a download.  The client should have
    // closed the socket at this point, but it is possible not all
    // data has been drained from the socket.
    //
    // At this point, it is an error condition for the socket to be
    // open, so an error should be returned if reading from the socket
    // would block.
    virtual void drain();

protected:
    UploadJob(internal::UploadJobImpl *p) UNITY_STORAGE_HIDDEN;
    internal::UploadJobImpl *p_ = nullptr;

    friend class internal::PendingJobs;
    friend class internal::ProviderInterface;
};

}
}
}
