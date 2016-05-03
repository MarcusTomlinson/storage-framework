#include <unity/storage/qt/client/Root.h>

#include <unity/storage/qt/client/internal/RootImpl.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

using namespace internal;

Root::Root(RootImpl* p)
    : Directory(p)
{
}

Root::~Root() = default;

Account* Root::account() const
{
    return static_cast<RootImpl*>(p_.get())->account();
}

QFuture<int64_t> Root::free_space_bytes() const
{
    return static_cast<RootImpl*>(p_.get())->free_space_bytes();
}

QFuture<int64_t> Root::used_space_bytes() const
{
    return static_cast<RootImpl*>(p_.get())->used_space_bytes();
}

QFuture<Item::UPtr> Root::get(QString native_identity) const
{
    return static_cast<RootImpl*>(p_.get())->get(native_identity);
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
