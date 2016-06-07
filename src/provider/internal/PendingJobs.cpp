#include <unity/storage/provider/internal/PendingJobs.h>
#include <unity/storage/provider/UploadJob.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

PendingJobs::PendingJobs(QObject *parent)
    : QObject(parent)
{
}
PendingJobs::~PendingJobs() = default;

void PendingJobs::add_upload(unique_ptr<UploadJob> &&job)
{
    assert(!job->sender_bus_name().empty());
    assert(uploads_.find(job->upload_id()) == uploads_.end());

    shared_ptr<UploadJob> j(std::move(job));
    uploads_.emplace(j->upload_id(), std::move(j));
}

std::shared_ptr<UploadJob> PendingJobs::get_upload(std::string const& upload_id)
{
    return uploads_.at(upload_id);
}

void PendingJobs::remove_upload(std::string const& upload_id)
{
    uploads_.erase(upload_id);
}

}
}
}
}
