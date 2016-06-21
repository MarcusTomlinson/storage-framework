#pragma once

#include <QFuture>
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

class RuntimeImpl
{
public:
    RuntimeImpl();
    ~RuntimeImpl();
    RuntimeImpl(RuntimeImpl const&) = delete;
    RuntimeImpl& operator=(RuntimeImpl const&) = delete;

    void shutdown();
    QFuture<QVector<std::shared_ptr<Account>>> accounts();

    void set_public_instance(std::weak_ptr<Runtime> p);

private:
    std::atomic_bool destroyed_;
    QVector<std::shared_ptr<Account>> accounts_;  // Immutable
    std::weak_ptr<Runtime> public_instance_;      // Immutable
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
