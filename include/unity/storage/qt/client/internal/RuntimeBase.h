#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <QFuture>
#pragma GCC diagnostic pop
#include <QVector>

#include <atomic>
#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Account;
class Runtime;

namespace internal
{

class RuntimeBase : public QObject
{
public:
    RuntimeBase();
    virtual ~RuntimeBase() = default;
    RuntimeBase(RuntimeBase const&) = delete;
    RuntimeBase& operator=(RuntimeBase const&) = delete;

    virtual void shutdown() = 0;
    virtual QFuture<QVector<std::shared_ptr<Account>>> accounts() = 0;

    void set_public_instance(std::weak_ptr<Runtime> p);

protected:
    std::atomic_bool destroyed_;
    QVector<std::shared_ptr<Account>> accounts_;  // Immutable once set
    std::weak_ptr<Runtime> public_instance_;      // Immutable once set
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
