#include <unity/storage/qt/client/internal/RootBase.h>

#include <unity/storage/qt/client/Exceptions.h>

using namespace std;

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

RootBase::RootBase(QString const& identity, weak_ptr<Account> const& account)
    : ItemBase(identity, ItemType::folder)
    , FolderBase(identity, ItemType::folder)
    , account_(account)
{
    assert(account.lock());
}

Account* RootBase::account() const
{
    if (auto acc = account_.lock())
    {
        return acc.get();
    }
    throw RuntimeDestroyedException();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
