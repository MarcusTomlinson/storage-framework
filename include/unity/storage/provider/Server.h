#pragma once

#include <unity/storage/visibility.h>

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
class CredentialsCache;
}

class ProviderBase;

class UNITY_STORAGE_EXPORT ServerBase
{
public:
    ServerBase();
    virtual ~ServerBase();

    void init(int& argc, char** argv);
    void run();

protected:
    virtual std::shared_ptr<ProviderBase> make_provider() = 0;
private:
    std::unique_ptr<QCoreApplication> app_;
    std::shared_ptr<internal::CredentialsCache> credentials_;
    std::unique_ptr<internal::ProviderInterface> interface_;
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
