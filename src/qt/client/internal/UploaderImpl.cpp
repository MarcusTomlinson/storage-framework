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

shared_ptr<StorageSocket> UploaderImpl::socket() const
{
    return shared_ptr<StorageSocket>();
}

QFuture<TransferState> UploaderImpl::finish_upload()
{
    return QFuture<TransferState>();
}

QFuture<void> UploaderImpl::cancel() noexcept
{
    return QFuture<void>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
