#pragma once

#include "Account.h"

namespace unity
{

namespace storage
{

namespace client
{

typedef std::vector<Account::UPtr> AccountList;

class AccountsResult
{
public:
    ~AccountsResult();
    AccountsResult(AccountsResult const&) = delete;
    AccountsResult& operator=(AccountsResult const&) = delete;
    AccountsResult(AccountsResult&&) = delete;
    AccountsResult& operator=(AccountsResult&&) = delete;

    AccountList get_accounts() const;

private:
    AccountsResult();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
