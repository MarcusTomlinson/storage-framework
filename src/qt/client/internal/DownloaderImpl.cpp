#include <unity/storage/qt/client/internal/DownloaderImpl.h>

#include <cassert>

using namespace std;

class QLocalSocket;

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

shared_ptr<QLocalSocket> DownloaderImpl::socket() const
{
    return shared_ptr<QLocalSocket>();
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
