#pragma once

#include <memory>

namespace unity
{

namespace storage
{

namespace client
{

class AccountsResult;

class Runtime
{
public:
    ~Runtime();
    Runtime(Runtime const&) = delete;
    Runtime& operator=(Runtime const&) = delete;
    Runtime(Runtime&&);
    Runtime& operator=(Runtime&&);

    typedef std::unique_ptr<Runtime> UPtr;

    static UPtr create();

    void get_accounts(std::function<void(AccountsResult const&)> accounts_callback);

private:
    Runtime();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
