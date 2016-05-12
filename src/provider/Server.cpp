#include <unity/storage/provider/Server.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/CredentialsCache.h>
#include <unity/storage/provider/internal/ProviderInterface.h>
#include <unity/storage/provider/internal/dbusmarshal.h>
#include "provideradaptor.h"

#include <QCoreApplication>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{

ServerBase::ServerBase()
{
}

ServerBase::~ServerBase() = default;

void ServerBase::init(int& argc, char** argv)
{
    app_.reset(new QCoreApplication(argc, argv));

    qDBusRegisterMetaType<Item>();
    qDBusRegisterMetaType<std::vector<Item>>();
    auto bus = QDBusConnection::sessionBus();

    credentials_ = make_shared<internal::CredentialsCache>(bus);

    // TODO: We should be creating multiple provider instances: one
    // for each account.
    interface_.reset(new internal::ProviderInterface(make_provider(), credentials_));
    // this instance is managed by Qt's parent/child memory management
    new ProviderAdaptor(interface_.get());

    bus.registerObject("/provider", interface_.get());

    // TODO: claim bus name
    qDebug() << "Bus unique name:" << bus.baseService();
}

void ServerBase::run()
{
    app_->exec();
}

}
}
}
