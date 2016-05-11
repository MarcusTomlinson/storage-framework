#include <unity/storage/qt/client/internal/RootImpl.h>

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

Account* RootImpl::account() const
{
    return nullptr;
}

QFuture<int64_t> RootImpl::free_space_bytes() const
{
    return QFuture<int64_t>();
}

QFuture<int64_t> RootImpl::used_space_bytes() const
{
    return QFuture<int64_t>();
}

QFuture<Item::SPtr> RootImpl::get(QString native_identity) const
{
    return QFuture<Item::SPtr>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
