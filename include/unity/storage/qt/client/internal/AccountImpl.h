#pragma once

#include <unity/storage/qt/client/Root.h>

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

class AccountImpl
{
public:
    AccountImpl() = default;
    ~AccountImpl() = default;
    AccountImpl(AccountImpl const&) = delete;
    AccountImpl& operator=(AccountImpl const&) = delete;

    QString owner() const;
    QString owner_id() const;
    QString description() const;
    QFuture<QVector<Root::UPtr>> get_roots() const;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
