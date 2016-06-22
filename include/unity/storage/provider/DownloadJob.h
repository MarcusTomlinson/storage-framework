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
namespace internal
{
class DownloadJobImpl;
}

class UNITY_STORAGE_EXPORT DownloadJob
{
public:
    DownloadJob(std::string const& download_id);
    virtual ~DownloadJob();

    std::string const& download_id() const;
    int write_socket() const;
    int take_read_socket();

    std::string const& sender_bus_name() const;
    void set_sender_bus_name(std::string const& bus_name);

    virtual boost::future<void> cancel() = 0;
    virtual boost::future<void> finish() = 0;

protected:
    DownloadJob(internal::DownloadJobImpl *p);
    internal::DownloadJobImpl *p_ = nullptr;
};

}
}
}
