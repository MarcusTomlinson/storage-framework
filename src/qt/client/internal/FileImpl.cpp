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

QFuture<shared_ptr<Uploader>> FileImpl::create_uploader(ConflictPolicy policy)
{
    return QFuture<shared_ptr<Uploader>>();
}

QFuture<shared_ptr<Downloader>> FileImpl::create_downloader()
{
    return QFuture<shared_ptr<Downloader>>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
