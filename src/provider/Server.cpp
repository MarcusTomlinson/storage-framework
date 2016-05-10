#include <unity/storage/provider/Server.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/ProviderInterface.h>
#include "provideradaptor.h"

#include <QCoreApplication>

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
    app.reset(new QCoreApplication(argc, argv));

    // TODO: We should be creating multiple provider instances: one
    // for each account.
    interface.reset(new internal::ProviderInterface(make_provider()));
    // this instance is managed by Qt's parent/child memory management
    new ProviderAdaptor(interface.get());

    auto bus = QDBusConnection::sessionBus();
    bus.registerObject("/provider", interface.get());

    // TODO: claim bus name
    qDebug() << "Bus unique name:" << bus.name();
}

void ServerBase::run()
{
    app->exec();
}

}
}
}
