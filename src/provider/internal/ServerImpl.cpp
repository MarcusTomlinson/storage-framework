#include <unity/storage/provider/internal/ServerImpl.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/dbusmarshal.h>
#include "provideradaptor.h"

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

ServerImpl::ServerImpl(ServerBase* server)
    : server_(server)
{
    qDBusRegisterMetaType<Item>();
    qDBusRegisterMetaType<std::vector<Item>>();
}

ServerImpl::~ServerImpl() = default;

void ServerImpl::init(int& argc, char **argv)
{
    app_.reset(new QCoreApplication(argc, argv));
    auto bus = QDBusConnection::sessionBus();
    credentials_ = make_shared<CredentialsCache>(bus);

    manager_.reset(new OnlineAccounts::Manager(""));
    connect(manager_.get(), &OnlineAccounts::Manager::ready,
                     this, &ServerImpl::account_manager_ready);
}

void ServerImpl::run()
{
    app_->exec();
}

void ServerImpl::account_manager_ready()
{
    auto bus = QDBusConnection::sessionBus();
    for (const auto& account : manager_->availableAccounts("google-drive-scope"))
    {
        qDebug() << "Found account" << account->id() << "for service" << account->serviceId();
        unique_ptr<ProviderInterface> iface(
            new ProviderInterface(server_->make_provider(), credentials_, account));
        // this instance is managed by Qt's parent/child memory management
        new ProviderAdaptor(iface.get());

        bus.registerObject(QStringLiteral("/provider/%1").arg(account->id()), iface.get());
        interfaces_.emplace(account->id(), std::move(iface));
    }

    // TODO: claim bus name
    qDebug() << "Bus unique name:" << bus.baseService();
}

}
}
}
}
