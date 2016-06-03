#include <unity/storage/qt/client/internal/UploaderImpl.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

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

shared_ptr<QLocalSocket> UploaderImpl::socket() const
{
    return shared_ptr<QLocalSocket>();
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
