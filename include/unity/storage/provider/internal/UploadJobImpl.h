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

#include <unity/storage/provider/ProviderBase.h>

#include <boost/thread/future.hpp>
#include <QObject>

#include <exception>
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

class UploadJobImpl : public QObject
{
    Q_OBJECT
public:
    explicit UploadJobImpl(std::string const& upload_id);
    virtual ~UploadJobImpl();

    std::string const& upload_id() const;
    int read_socket() const;
    int take_write_socket();

    std::string const& sender_bus_name() const;
    void set_sender_bus_name(std::string const& bus_name);

    void report_error(std::exception_ptr p);
    boost::future<Item> finish(UploadJob& job);
    boost::future<void> cancel(UploadJob& job);

public Q_SLOTS:
    virtual void complete_init();

protected:
    std::string const upload_id_;
    int read_socket_ = -1;
    int write_socket_ = -1;
    std::string sender_bus_name_;

    std::mutex completion_lock_;
    bool completed_ = false;
    boost::promise<Item> completion_promise_;

    Q_DISABLE_COPY(UploadJobImpl)
};

}
}
}
}
