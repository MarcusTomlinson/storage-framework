#pragma once

#include <unity/storage/provider/visibility.h>

#include <memory>
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

namespace internal
{
class ServerImpl;
}

class ProviderBase;

class STORAGE_PROVIDER_EXPORT ServerBase
{
public:
    ServerBase(std::string const& bus_name, std::string const& account_service_id);
    virtual ~ServerBase();

    void init(int& argc, char** argv);
    void run();

protected:
    virtual std::shared_ptr<ProviderBase> make_provider() = 0;
private:
    std::unique_ptr<internal::ServerImpl> p_;

    friend class internal::ServerImpl;
};

template <typename T>
class Server : public ServerBase
{
public:
    using ServerBase::ServerBase;
protected:
    std::shared_ptr<ProviderBase> make_provider() override {
        return std::make_shared<T>();
    }
};

}
}
}
