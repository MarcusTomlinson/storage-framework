#pragma once

#include <memory>
#include <vector>

namespace unity
{

namespace storage
{

namespace client
{

class RootsResult;

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

    std::string owner() const;
    std::string owner_id() const;
    std::string description() const;

    // TODO: Will almost certainly need more here. Description? Other details?

    /**
    \brief Returns the root directories for the account.

    An account can have more than one root directory (for providers that support the concept of multiple drives).
    */
    void get_roots(std::function<void(RootsResult const&)> roots_callback) const;
};

}  // namespace client

}  // namespace storage

}  // namespace unity
