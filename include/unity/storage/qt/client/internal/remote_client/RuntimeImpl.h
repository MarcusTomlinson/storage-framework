#pragma once

#include <unity/storage/qt/client/internal/RuntimeBase.h>

#include <QDBusConnection>
#include <QFuture>
#include <QVector>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{

class RuntimeImpl : public RuntimeBase
{
public:
    RuntimeImpl();
    virtual ~RuntimeImpl();

    virtual void shutdown() override;
    virtual QFuture<QVector<std::shared_ptr<Account>>> accounts() override;

    QDBusConnection& connection();

private:
    QDBusConnection conn_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
