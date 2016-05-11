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

QFuture<shared_ptr<QLocalSocket>> DownloaderImpl::socket() const
{
    return QFuture<shared_ptr<QLocalSocket>>();
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
