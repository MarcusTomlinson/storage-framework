#pragma once

#include <unity/storage/qt/client/Root.h>

namespace unity
{

namespace storage
{

namespace qt
{

namespace client
{

/**
\brief Class that represents an account.
*/

class Account
{
public:
    ~Account();
    Account(Account const&) = delete;
    Account& operator=(Account const&) = delete;
    Account(Account&&);
    Account& operator=(Account&&);

    typedef std::unique_ptr<Account> UPtr;

    QString owner() const;
    QString owner_id() const;
    QString description() const;

    // TODO: Will almost certainly need more here. Other details?

    /**
    \brief Returns the root directories for the account.

    An account can have more than one root directory (for providers that support the concept of multiple drives).
    */
    QFuture<QVector<Root::UPtr>> get_roots() const;
};

}  // namespace client

}  // namespace qt

}  // namespace storage

}  // namespace unity
