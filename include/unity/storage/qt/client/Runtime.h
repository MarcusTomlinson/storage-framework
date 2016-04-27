#pragma once

#include <unity/storage/qt/client/Account.h>

namespace unity
{

namespace storage
{

namespace qt
{

namespace client
{

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

    QFuture<QVector<Account::UPtr>> get_accounts();

private:
    Runtime();
};

}  // namespace client

}  // namespace qt

}  // namespace storage

}  // namespace unity
