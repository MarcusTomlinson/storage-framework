#include "DBusEnvironment.h"

#include <testsetup.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbustest/QProcessDBusService.h>

namespace {
char const ACCOUNTS_BUS_NAME[] = "com.ubuntu.OnlineAccounts.Manager";
char const FAKE_ACCOUNTS_SERVICE[] = TEST_SRC_DIR "/utils/fake-online-accounts-daemon.py";
}

DBusEnvironment::DBusEnvironment()
{
    runner_.reset(new QtDBusTest::DBusTestRunner());
    accounts_service_.reset(new QtDBusTest::QProcessDBusService(
                                ACCOUNTS_BUS_NAME, QDBusConnection::SessionBus,
                                FAKE_ACCOUNTS_SERVICE, {}));
    runner_->registerService(accounts_service_);
    runner_->startServices();
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

QDBusConnection const& DBusEnvironment::connection()
{
    return runner_->sessionConnection();
}

QProcess& DBusEnvironment::accounts_service_process()
{
    // We need a non-const version to access waitForFinished()
    return const_cast<QProcess&>(accounts_service_->underlyingProcess());
}
