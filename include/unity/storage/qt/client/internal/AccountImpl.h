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
class Root;
class Runtime;

namespace internal
{

class AccountImpl
{
public:
    AccountImpl(QString const& owner,
                QString const& owner_id,
                QString const& description);
    ~AccountImpl() = default;
    AccountImpl(AccountImpl const&) = delete;
    AccountImpl& operator=(AccountImpl const&) = delete;

    Runtime* runtime() const;
    QString owner() const;
    QString owner_id() const;
    QString description() const;
    QFuture<QVector<std::shared_ptr<Root>>> get_roots();

    void set_runtime(std::weak_ptr<Runtime> p);
    void set_public_instance(std::weak_ptr<Account> p);

private:
    QString owner_;
    QString owner_id_;
    QString description_;
    QVector<std::shared_ptr<Root>> roots_;
    std::weak_ptr<Runtime> runtime_;
    std::weak_ptr<Account> public_instance_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
