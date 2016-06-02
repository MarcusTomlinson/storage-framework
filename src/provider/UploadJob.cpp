#include <unity/storage/provider/UploadJob.h>
#include <unity/storage/provider/internal/UploadJobImpl.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{

UploadJob::UploadJob(internal::UploadJobImpl *p)
    : p_(p)
{
}

UploadJob::UploadJob(string const& upload_id)
    : UploadJob(new internal::UploadJobImpl(upload_id))
{
}

UploadJob::~UploadJob()
{
    if (p_)
    {
        p_->deleteLater();
    }
}

string const& UploadJob::upload_id() const
{
    return p_->upload_id();
}

int UploadJob::read_socket() const
{
    return p_->read_socket();
}

int UploadJob::take_write_socket()
{
    return p_->take_write_socket();
}

}
}
}
