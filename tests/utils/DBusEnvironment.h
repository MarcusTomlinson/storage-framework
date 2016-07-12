#pragma once

#include <QDBusConnection>
#include <QProcess>
#include <QSharedPointer>
#include <memory>

namespace QtDBusTest
{
class DBusTestRunner;
class QProcessDBusService;
}

class DBusEnvironment final {
public:
    DBusEnvironment();
    ~DBusEnvironment();

    QDBusConnection const& connection();

    QProcess& accounts_service_process();

private:
    std::unique_ptr<QtDBusTest::DBusTestRunner> runner_;
    QSharedPointer<QtDBusTest::QProcessDBusService> accounts_service_;
};
