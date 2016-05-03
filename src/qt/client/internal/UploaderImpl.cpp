#include <unity/storage/qt/client/internal/UploaderImpl.h>

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

namespace internal
{

QFuture<int> UploaderImpl::fd() const
{
    return QFuture<int>();
}

QFuture<void> UploaderImpl::close()
{
    return QFuture<void>();
}

QFuture<void> UploaderImpl::cancel()
{
    return QFuture<void>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
