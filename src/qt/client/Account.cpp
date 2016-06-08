#include <unity/storage/qt/client/Account.h>

#include <unity/storage/qt/client/internal/AccountBase.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

Account::Account(internal::AccountBase* p)
    : p_(p)
{
    assert(p != nullptr);
}

Account::~Account() = default;

Runtime* Account::runtime() const
{
    return p_->runtime();
}

QString Account::owner() const
{
    return p_->owner();
}

QString Account::owner_id() const
{
    return p_->owner_id();
}

QString Account::description() const
{
    return p_->description();
}

QFuture<QVector<std::shared_ptr<Root>>> Account::roots() const
{
    return p_->roots();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
