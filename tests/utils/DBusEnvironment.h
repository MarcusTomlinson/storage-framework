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

    QDBusConnection const& connection() const;

    void add_demo_provider(char const* service_id);
    void start_services();

    QProcess& accounts_service_process();

private:
    std::unique_ptr<QtDBusTest::DBusTestRunner> runner_;
    QSharedPointer<QtDBusTest::QProcessDBusService> accounts_service_;
    QSharedPointer<QtDBusTest::QProcessDBusService> demo_provider_;
};
