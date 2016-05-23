#pragma once

#include <memory>

class QCoreApplication;

namespace unity
{
namespace storage
{
namespace provider
{

class ServerBase;

namespace internal
{

class CredentialsCache;
class ProviderInterface;

class ServerImpl {
public:
    ServerImpl(ServerBase* server);
    ~ServerImpl();

    void init(int& argc, char **argv);
    void run();

private:
    ServerBase* const server_;
    std::unique_ptr<QCoreApplication> app_;
    std::shared_ptr<CredentialsCache> credentials_;
    std::unique_ptr<ProviderInterface> interface_;
};

}
}
}
}
