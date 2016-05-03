#include <unity/storage/qt/client/Account.h>

#include <unity/storage/qt/client/internal/AccountImpl.h>

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

Account::Account(internal::AccountImpl* p)
    : p_(p)
{
    assert(p != nullptr);
}

Account::~Account() = default;

QString Account::owner() const
{
    return p_->owner();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
