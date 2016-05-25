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

class StorageSocket;

namespace internal
{

shared_ptr<StorageSocket> DownloaderImpl::socket() const
{
    return shared_ptr<StorageSocket>();
}

QFuture<TransferState> DownloaderImpl::finish_download()
{
    return QFuture<TransferState>();
}

QFuture<void> DownloaderImpl::cancel() noexcept
{
    return QFuture<void>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
