#include <unity/storage/provider/internal/DownloadJobImpl.h>
#include <unity/storage/provider/DownloadJob.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include <stdexcept>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

DownloadJobImpl::DownloadJobImpl(std::string const& download_id)
    : download_id_(download_id)
{
    int socks[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socks) < 0)
    {
        throw runtime_error("could not create socketpair");
    }
    read_socket_ = socks[0];
    write_socket_ = socks[1];
    if (shutdown(read_socket_, SHUT_WR) < 0)
    {
        throw runtime_error("Could not shut down write channel on read socket");
    }
    if (shutdown(write_socket_, SHUT_RD) < 0)
    {
        throw runtime_error("Could not shut down read channel on write socket");
    }
}

DownloadJobImpl::~DownloadJobImpl()
{
    if (read_socket_ >= 0)
    {
        close(read_socket_);
    }
    if (write_socket_ >= 0)
    {
        close(write_socket_);
    }
}

void DownloadJobImpl::complete_init()
{
}

std::string const& DownloadJobImpl::download_id() const
{
    return download_id_;
}

int DownloadJobImpl::write_socket() const
{
    return write_socket_;
}

int DownloadJobImpl::take_read_socket()
{
    assert(read_socket_ >= 0);
    int sock = read_socket_;
    read_socket_ = -1;
    return sock;
}

string const& DownloadJobImpl::sender_bus_name() const
{
    return sender_bus_name_;
}

void DownloadJobImpl::set_sender_bus_name(string const& bus_name)
{
    assert(bus_name[0] == ':');
    sender_bus_name_ = bus_name;
}

void DownloadJobImpl::report_complete()
{
    if (write_socket_ >= 0)
    {
        close(write_socket_);
        write_socket_ = -1;
    }

    lock_guard<mutex> guard(completion_lock_);
    completed_ = true;
    completion_promise_.set_value();
}

void DownloadJobImpl::report_error(std::exception_ptr p)
{
    if (write_socket_ >= 0)
    {
        close(write_socket_);
        write_socket_ = -1;
    }

    lock_guard<mutex> guard(completion_lock_);
    completed_ = true;
    completion_promise_.set_exception(p);
}

boost::future<void> DownloadJobImpl::finish(DownloadJob& job)
{
    lock_guard<mutex> guard(completion_lock_);
    if (completed_)
    {
        return completion_promise_.get_future();
    }
    return job.finish();
}

boost::future<void> DownloadJobImpl::cancel(DownloadJob& job)
{
    lock_guard<mutex> guard(completion_lock_);
    if (completed_)
    {
        return boost::make_ready_future();
    }
    return job.cancel();
}

}
}
}
}
