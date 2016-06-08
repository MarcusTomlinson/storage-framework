#pragma once

#include <unity/storage/qt/client/internal/RuntimeBase.h>

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

class RuntimeImpl : public virtual RuntimeBase
{
public:
    RuntimeImpl();
    virtual ~RuntimeImpl();

    virtual void shutdown() override;
    virtual QFuture<QVector<std::shared_ptr<Account>>> accounts() override;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
