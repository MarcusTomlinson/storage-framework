#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
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
class Root;
class Runtime;

namespace internal
{

class AccountBase
{
public:
    AccountBase(std::weak_ptr<Runtime> const& runtime);
    virtual ~AccountBase() = default;
    AccountBase(AccountBase const&) = delete;
    AccountBase& operator=(AccountBase const&) = delete;

    Runtime* runtime() const;
    virtual QString owner() const = 0;
    virtual QString owner_id() const = 0;
    virtual QString description() const = 0;
    virtual QFuture<QVector<std::shared_ptr<Root>>> roots() = 0;

    void set_public_instance(std::weak_ptr<Account> const& p);

protected:
    std::weak_ptr<Runtime> runtime_;          // Immutable once set
    std::weak_ptr<Account> public_instance_;  // Immutable once set
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
