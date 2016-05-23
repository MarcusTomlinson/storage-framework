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

ServerBase::ServerBase()
    : p_(new internal::ServerImpl(this))
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
