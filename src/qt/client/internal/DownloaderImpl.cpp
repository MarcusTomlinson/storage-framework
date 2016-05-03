#include <unity/storage/qt/client/internal/DownloaderImpl.h>

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

QFuture<int> DownloaderImpl::fd() const
{
    return QFuture<int>();
}

QFuture<void> DownloaderImpl::close()
{
    return QFuture<void>();
}

QFuture<void> DownloaderImpl::cancel()
{
    return QFuture<void>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
