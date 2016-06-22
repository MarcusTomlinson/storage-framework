#include <unity/storage/provider/DownloadJob.h>
#include <unity/storage/provider/internal/DownloadJobImpl.h>

#include <QCoreApplication>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{

DownloadJob::DownloadJob(internal::DownloadJobImpl *p)
    : p_(p)
{
    // We may be created by user code running in some other thread:
    // make sure our events are processed on the event loop thread,
    // and then let the class complete its initialisation on that
    // thread.
    p_->moveToThread(QCoreApplication::instance()->thread());
    QMetaObject::invokeMethod(p_, "complete_init", Qt::QueuedConnection);
}

DownloadJob::DownloadJob(string const& download_id)
    : DownloadJob(new internal::DownloadJobImpl(download_id))
{
}

DownloadJob::~DownloadJob()
{
    if (p_)
    {
        p_->deleteLater();
    }
}

string const& DownloadJob::download_id() const
{
    return p_->download_id();
}

int DownloadJob::write_socket() const
{
    return p_->write_socket();
}

int DownloadJob::take_read_socket()
{
    return p_->take_read_socket();
}

string const& DownloadJob::sender_bus_name() const
{
    return p_->sender_bus_name();
}

void DownloadJob::set_sender_bus_name(string const& bus_name)
{
    p_->set_sender_bus_name(bus_name);
}


}
}
}
