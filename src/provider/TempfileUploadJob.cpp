#include <unity/storage/provider/TempfileUploadJob.h>
#include <unity/storage/provider/internal/TempfileUploadJobImpl.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{

TempfileUploadJob::TempfileUploadJob(internal::TempfileUploadJobImpl *p)
    : UploadJob(p)
{
}

TempfileUploadJob::TempfileUploadJob(string const& upload_id)
    : TempfileUploadJob(new internal::TempfileUploadJobImpl(upload_id))
{
}

TempfileUploadJob::~TempfileUploadJob() = default;

string TempfileUploadJob::file_name() const
{
    return static_cast<internal::TempfileUploadJobImpl*>(p_)->file_name();
}

}
}
}
