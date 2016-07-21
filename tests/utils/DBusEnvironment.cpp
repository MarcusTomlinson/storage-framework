#include "DBusEnvironment.h"

#include <testsetup.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbustest/QProcessDBusService.h>

namespace {
char const ACCOUNTS_BUS_NAME[] = "com.ubuntu.OnlineAccounts.Manager";
char const FAKE_ACCOUNTS_SERVICE[] = TEST_SRC_DIR "/utils/fake-online-accounts-daemon.py";

char const DEMO_PROVIDER_BUS_NAME[] = "com.canonical.StorageFramework.Provider.ProviderTest";
char const DEMO_PROVIDER_SERVICE[] = TEST_BIN_DIR "/../demo/provider_test/provider-test";
}

DBusEnvironment::DBusEnvironment()
{
    runner_.reset(new QtDBusTest::DBusTestRunner());
    accounts_service_.reset(new QtDBusTest::QProcessDBusService(
                                ACCOUNTS_BUS_NAME, QDBusConnection::SessionBus,
                                FAKE_ACCOUNTS_SERVICE, {}));
    runner_->registerService(accounts_service_);
}

DBusEnvironment::~DBusEnvironment()
{
#if 0
    // TODO: implement graceful shutdown
    if (accounts_service_process().state() == QProcess::Running)
    {
        
    }
#endif
    runner_.reset();
}

QDBusConnection const& DBusEnvironment::connection() const
{
    return runner_->sessionConnection();
}

void DBusEnvironment::add_demo_provider(char const* service_id)
{
    demo_provider_.reset(
        new QtDBusTest::QProcessDBusService(
            DEMO_PROVIDER_BUS_NAME, QDBusConnection::SessionBus,
            DEMO_PROVIDER_SERVICE, {service_id}));
    runner_->registerService(demo_provider_);
}

void DBusEnvironment::start_services()
{
    runner_->startServices();
}


QProcess& DBusEnvironment::accounts_service_process()
{
    // We need a non-const version to access waitForFinished()
    return const_cast<QProcess&>(accounts_service_->underlyingProcess());
}
