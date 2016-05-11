#pragma once

#include <QFuture>
#include <QVector>

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
    RuntimeImpl() = default;
    ~RuntimeImpl() = default;
    RuntimeImpl(RuntimeImpl const&) = delete;
    RuntimeImpl& operator=(RuntimeImpl const&) = delete;

    QFuture<QVector<std::shared_ptr<Account>>> get_accounts();

    void set_public_instance(std::weak_ptr<Runtime> p);

private:
    bool destroyed_ = false;
    std::weak_ptr<Runtime> public_instance_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
