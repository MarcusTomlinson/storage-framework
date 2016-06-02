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

protected:
    UploadJob(internal::UploadJobImpl *p);
    internal::UploadJobImpl *p_ = nullptr;
};

}
}
}
