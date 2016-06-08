#include <unity/storage/qt/client/Root.h>

#include <unity/storage/qt/client/internal/RootBase.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

using namespace internal;

Root::Root(RootBase* p)
    : Folder(p)
{
}

Root::~Root() = default;

Account* Root::account() const
{
    return dynamic_cast<RootBase*>(p_.get())->account();
}

QFuture<int64_t> Root::free_space_bytes() const
{
    return dynamic_cast<RootBase*>(p_.get())->free_space_bytes();
}

QFuture<int64_t> Root::used_space_bytes() const
{
    return dynamic_cast<RootBase*>(p_.get())->used_space_bytes();
}

QFuture<Item::SPtr> Root::get(QString native_identity) const
{
    return dynamic_cast<RootBase*>(p_.get())->get(native_identity);
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
