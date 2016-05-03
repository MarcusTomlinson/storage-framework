#include <unity/storage/qt/client/internal/FileImpl.h>

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

int64_t FileImpl::size() const
{
    return 0;
}

QFuture<Uploader::UPtr> FileImpl::create_uploader(ConflictPolicy policy)
{
    return QFuture<Uploader::UPtr>();
}

QFuture<Downloader::UPtr> FileImpl::create_downloader()
{
    return QFuture<Downloader::UPtr>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
