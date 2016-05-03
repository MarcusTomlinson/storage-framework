#pragma once

#include <unity/storage/qt/client/Account.h>

#include <QFuture>
#include <QVector>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{

class RuntimeImpl
{
public:
    RuntimeImpl() = default;
    ~RuntimeImpl() = default;
    RuntimeImpl(RuntimeImpl const&) = delete;
    RuntimeImpl& operator=(RuntimeImpl const&) = delete;

    QFuture<QVector<Account::UPtr>> get_accounts();
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
