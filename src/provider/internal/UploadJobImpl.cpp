#include <unity/storage/provider/internal/UploadJobImpl.h>

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

UploadJobImpl::UploadJobImpl(std::string const& upload_id)
    : upload_id_(upload_id)
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

UploadJobImpl::~UploadJobImpl()
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

std::string const& UploadJobImpl::upload_id() const
{
    return upload_id_;
}

int UploadJobImpl::read_socket() const
{
    return read_socket_;
}

int UploadJobImpl::take_write_socket()
{
    assert(write_socket_ >= 0);
    int sock = write_socket_;
    write_socket_ = -1;
    return sock;
}

string const& UploadJobImpl::sender_bus_name() const
{
    return sender_bus_name_;
}

void UploadJobImpl::set_sender_bus_name(string const& bus_name)
{
    assert(bus_name[0] == ':');
    sender_bus_name_ = bus_name;
}

}
}
}
}
