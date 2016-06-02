#pragma once

#include <unity/storage/provider/UploadJob.h>
#include <unity/storage/provider/visibility.h>

#include <string>
#include <memory>

namespace unity
{
namespace storage
{
namespace provider
{

namespace internal
{
class TempfileUploadJobImpl;
}

class STORAGE_PROVIDER_EXPORT TempfileUploadJob : public UploadJob
{
public:
    TempfileUploadJob(std::string const& upload_id);
    virtual ~TempfileUploadJob();

    std::string file_name() const;

protected:
    TempfileUploadJob(internal::TempfileUploadJobImpl *p);
};

}
}
}
