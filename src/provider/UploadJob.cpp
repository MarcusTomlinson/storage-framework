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
    // make sure our events are processed on the event loop thread,
    // and then let the class complete its initialisation on that
    // thread.
    p_->moveToThread(QCoreApplication::instance()->thread());
    QMetaObject::invokeMethod(p_, "complete_init", Qt::QueuedConnection);
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

}
}
}
