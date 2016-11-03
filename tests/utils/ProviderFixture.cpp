/*
 * Copyright (C) 2016 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#include "ProviderFixture.h"

#include <unity/storage/internal/dbus_error.h>

#include <OnlineAccounts/Account>
#include <OnlineAccounts/Manager>
#include <QSignalSpy>
#include <QThread>

#include <condition_variable>
#include <mutex>

using namespace unity::storage::provider;
using namespace std;

namespace
{

const auto SERVICE_CONNECTION_NAME = QStringLiteral("service-session-bus");
const auto OBJECT_PATH = QStringLiteral("/provider");

}  // namespace

class ProviderFixture::ServiceThread : public QThread
{
public:
    ServiceThread(QString const& bus_address, QObject *parent=nullptr);

    void wait_for_ready();
    void set_provider(shared_ptr<ProviderBase> const& provider);
    void join();

    QString bus_name() const;

protected:
    void run() override;

private:
    QString const bus_address_;

    mutex mutex_;
    condition_variable connection_ready_cond_;
    condition_variable provider_cond_;
    condition_variable provider_ready_cond_;
    bool connection_ready_ = false;
    bool provider_set_ = false;
    shared_ptr<ProviderBase> provider_;
    bool provider_ready_ = false;

    unique_ptr<QDBusConnection> connection_;
    unique_ptr<OnlineAccounts::Manager> account_manager_;
    unique_ptr<unity::storage::provider::testing::TestServer> test_server_;

};

ProviderFixture::ServiceThread::ServiceThread(QString const& bus_address,
                                              QObject *parent)
    : QThread(parent), bus_address_(bus_address)
{
}

void ProviderFixture::ServiceThread::wait_for_ready()
{
    unique_lock<mutex> lock(mutex_);
    while (!connection_ready_)
    {
        connection_ready_cond_.wait(lock);
    }
}

void ProviderFixture::ServiceThread::set_provider(shared_ptr<ProviderBase> const& provider)
{
    {
        unique_lock<mutex> lock(mutex_);
        provider_set_ = true;
        provider_ = provider;
        provider_cond_.notify_all();
    }
    {
        unique_lock<mutex> lock(mutex_);
        while (!provider_ready_)
        {
            provider_ready_cond_.wait(lock);
        }
    }
}

void ProviderFixture::ServiceThread::join()
{
    // First make sure we unblock the thread waiting for the provider
    {
        unique_lock<mutex> lock(mutex_);
        provider_set_ = true;
        provider_cond_.notify_all();
    }
    // Next quit the event loop if it is running
    quit();
    // Finally, wait for thread to exit
    wait();
}

QString ProviderFixture::ServiceThread::bus_name() const
{
    if (!connection_)
    {
        return QString();
    }
    return connection_->baseService();
}

void ProviderFixture::ServiceThread::run()
{
    printf("Service thread: starting\n");
    connection_.reset(new QDBusConnection(QDBusConnection::connectToBus(
        bus_address_, SERVICE_CONNECTION_NAME)));

    account_manager_.reset(new OnlineAccounts::Manager("", *connection_));
    account_manager_->waitForReady();
    OnlineAccounts::Account* account = account_manager_->account(2, "oauth2-service");
    ASSERT_NE(nullptr, account);

    printf("Service thread: notifying that connection is ready\n");
    // Notify that we've successfully connected to the bus
    {
        unique_lock<mutex> lock(mutex_);
        connection_ready_ = true;
        connection_ready_cond_.notify_all();
    }

    printf("Service thread: waiting for ProviderBase\n");
    // Now wait for main thread to set the provider
    shared_ptr<ProviderBase> provider;
    {
        unique_lock<mutex> lock(mutex_);
        while (!provider_set_)
        {
            provider_cond_.wait(lock);
        }
        provider = provider_;
    }

    // Bypass the event loop if we never get a provider
    if (provider)
    {
        printf("Service thread: got a provider\n");
        test_server_.reset(
            new unity::storage::provider::testing::TestServer(
                provider, account, *connection_, OBJECT_PATH.toStdString()));
        printf("Service thread: notifying that provider is ready\n");
        {
            unique_lock<mutex> lock(mutex_);
            provider_ready_ = true;
            provider_ready_cond_.notify_all();
        }

        printf("Service thread: running event loop\n");
        // Run the thread's event loop
        exec();
    }

    printf("Service thread: cleaning up\n");
    test_server_.reset();
    account_manager_.reset();
    connection_.reset();
    QDBusConnection::disconnectFromBus(SERVICE_CONNECTION_NAME);
}

ProviderFixture::ProviderFixture() = default;
ProviderFixture::~ProviderFixture() = default;

void ProviderFixture::SetUp()
{
    dbus_.reset(new DBusEnvironment);
    dbus_->start_services();

    service_thread_.reset(new ServiceThread(dbus_->busAddress()));
    service_thread_->start();
    service_thread_->wait_for_ready();
}

void ProviderFixture::TearDown()
{
    if (service_thread_)
    {
        service_thread_->join();
    }
    service_thread_.reset();
    dbus_.reset();
}

QDBusConnection const& ProviderFixture::connection() const
{
    return dbus_->connection();
}

void ProviderFixture::set_provider(unique_ptr<ProviderBase>&& provider)
{
    service_thread_->set_provider(move(provider));
}

void ProviderFixture::wait_for(QDBusPendingCall const& call)
{
    QDBusPendingCallWatcher watcher(call);
    QSignalSpy spy(&watcher, &QDBusPendingCallWatcher::finished);
    ASSERT_TRUE(spy.wait());
}

QString ProviderFixture::bus_name() const
{
    return service_thread_->bus_name();
}

QString ProviderFixture::object_path() const
{
    return OBJECT_PATH;
}
