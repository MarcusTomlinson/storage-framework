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

    virtual boost::future<void> cancel() = 0;
    virtual boost::future<Item> finish() = 0;

protected:
    UploadJob(internal::UploadJobImpl *p);
    internal::UploadJobImpl *p_ = nullptr;

    friend class internal::PendingJobs;
    friend class internal::ProviderInterface;
};

}
}
}
