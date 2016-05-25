#include <unity/storage/provider/Server.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/ServerImpl.h>


using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{

ServerBase::ServerBase(std::string const& bus_name, std::string const& account_service_id)
    : p_(new internal::ServerImpl(this, bus_name, account_service_id))
{
}

ServerBase::~ServerBase() = default;

void ServerBase::init(int& argc, char** argv)
{
    p_->init(argc, argv);
}

void ServerBase::run()
{
    p_->run();
}

}
}
}
