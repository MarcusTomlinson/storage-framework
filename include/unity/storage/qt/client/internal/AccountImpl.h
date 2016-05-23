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
    QFuture<QVector<std::shared_ptr<Root>>> roots();

    void set_runtime(std::weak_ptr<Runtime> p);
    void set_public_instance(std::weak_ptr<Account> p);

    class RecursiveCopyGuard
    {
    public:
        RecursiveCopyGuard(AccountImpl*);
        ~RecursiveCopyGuard();
        RecursiveCopyGuard(RecursiveCopyGuard&&) = default;
        RecursiveCopyGuard& operator=(RecursiveCopyGuard&&) = default;

    private:
        AccountImpl* account_;
    };

    RecursiveCopyGuard get_copy_guard();

private:
    QString owner_;
    QString owner_id_;
    QString description_;
    QVector<std::shared_ptr<Root>> roots_;
    std::weak_ptr<Runtime> runtime_;
    std::weak_ptr<Account> public_instance_;
    bool copy_in_progress_ = false;

    friend class RecursiveCopyGuard;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
