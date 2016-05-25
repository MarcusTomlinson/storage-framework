#pragma once

#include <unity/storage/provider/Server.h>
#include <unity/storage/provider/internal/CredentialsCache.h>
#include <unity/storage/provider/internal/ProviderInterface.h>

#include <OnlineAccounts/Manager>
#include <OnlineAccounts/Account>
#include <QObject>
#include <QCoreApplication>
#include <QDBusConnection>

#include <map>
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

class ServerImpl : public QObject {
    Q_OBJECT
public:
    ServerImpl(ServerBase* server, std::string const& bus_name, std::string const& account_service_id);
    ~ServerImpl();

    void init(int& argc, char **argv);
    void run();

private Q_SLOTS:
    void account_manager_ready();

private:
    ServerBase* const server_;
    std::string const bus_name_;
    std::string const service_id_;

    std::unique_ptr<QCoreApplication> app_;
    std::unique_ptr<OnlineAccounts::Manager> manager_;
    std::shared_ptr<CredentialsCache> credentials_;
    std::map<OnlineAccounts::AccountId,std::unique_ptr<ProviderInterface>> interfaces_;
};

}
}
}
}
