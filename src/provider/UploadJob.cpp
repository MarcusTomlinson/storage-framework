#include <unity/storage/provider/UploadJob.h>
#include <unity/storage/provider/internal/UploadJobImpl.h>

#include <QCoreApplication>

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
    // We may be created by user code running in some other thread:
    // make sure our events are processed on the event loop thread.
    p_->moveToThread(QCoreApplication::instance()->thread());
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

string const& UploadJob::sender_bus_name() const
{
    return p_->sender_bus_name();
}

void UploadJob::set_sender_bus_name(string const& bus_name)
{
    p_->set_sender_bus_name(bus_name);
}


}
}
}
