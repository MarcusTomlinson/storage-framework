#pragma once

#include <unity/storage/provider/visibility.h>

#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

namespace internal
{
class UploadJobImpl;
}

class STORAGE_PROVIDER_EXPORT UploadJob
{
public:
    UploadJob(std::string const& upload_id);
    virtual ~UploadJob();

    std::string const& upload_id() const;
    int read_socket() const;
    int take_write_socket();

    std::string const& sender_bus_name() const;
    void set_sender_bus_name(std::string const& bus_name);

protected:
    UploadJob(internal::UploadJobImpl *p);
    internal::UploadJobImpl *p_ = nullptr;
};

}
}
}
