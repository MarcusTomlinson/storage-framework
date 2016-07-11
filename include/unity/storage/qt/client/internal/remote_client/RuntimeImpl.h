#pragma once

#include <unity/storage/qt/client/internal/RuntimeBase.h>

#include <OnlineAccounts/Manager>
#include <QDBusConnection>
#include <QFuture>
#include <QTimer>
#include <QVector>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Account;

namespace internal
{
namespace remote_client
{

class RuntimeImpl : public RuntimeBase
{
    Q_OBJECT

public:
    RuntimeImpl();
    virtual ~RuntimeImpl();

    virtual void shutdown() override;
    virtual QFuture<QVector<std::shared_ptr<Account>>> accounts() override;

    QDBusConnection& connection();

private Q_SLOTS:
    virtual void manager_ready();
    virtual void timeout();

private:
    QDBusConnection conn_;
    std::unique_ptr<OnlineAccounts::Manager> manager_;  // TODO: Hack until we can use the registry
    QTimer timer_;
    QFutureInterface<QVector<std::shared_ptr<Account>>> qf_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
