#pragma once

#include <memory>

class QCoreApplication;

namespace unity
{
namespace storage
{
namespace provider
{

namespace internal
{
class ProviderInterface;
}

class ProviderBase;

class __attribute__((visibility("default"))) ServerBase
{
public:
    ServerBase();
    virtual ~ServerBase();

    void init(int& argc, char** argv);
    void run();

protected:
    virtual std::shared_ptr<ProviderBase> make_provider() = 0;
private:
    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<internal::ProviderInterface> interface;
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
