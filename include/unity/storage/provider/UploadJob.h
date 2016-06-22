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

class Item;

namespace internal
{
class UploadJobImpl;
}

class UNITY_STORAGE_EXPORT UploadJob
{
public:
    UploadJob(std::string const& upload_id);
    virtual ~UploadJob();

    std::string const& upload_id() const;
    int read_socket() const;
    int take_write_socket();

    std::string const& sender_bus_name() const;
    void set_sender_bus_name(std::string const& bus_name);

    virtual boost::future<void> cancel() = 0;
    virtual boost::future<Item> finish() = 0;

protected:
    UploadJob(internal::UploadJobImpl *p);
    internal::UploadJobImpl *p_ = nullptr;
};

}
}
}
