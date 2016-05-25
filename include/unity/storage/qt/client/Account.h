#pragma once

#include <unity/storage/visibility.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop
#include <QString>
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

class Runtime;

class Root;

namespace internal
{

class AccountImpl;
class ItemImpl;
class RuntimeImpl;

}

/**
\brief Class that represents an account.
*/
class UNITY_STORAGE_EXPORT Account
{
public:
    ~Account();
    Account(Account const&) = delete;
    Account& operator=(Account const&) = delete;
    Account(Account&&);
    Account& operator=(Account&&);

    typedef std::shared_ptr<Account> SPtr;

    Runtime* runtime() const;

    QString owner() const;
    QString owner_id() const;
    QString description() const;

    // TODO: Will almost certainly need more here. Other details?

    /**
    \brief Returns the root directories for the account.

    An account can have more than one root directory (for providers that support the concept of multiple drives).
    */
    QFuture<QVector<std::shared_ptr<Root>>> roots() const;

private:
    Account(internal::AccountImpl*);

    std::unique_ptr<internal::AccountImpl> p_;

    friend class internal::ItemImpl;
    friend class internal::RuntimeImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
