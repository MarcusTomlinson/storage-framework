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

QFuture<shared_ptr<QLocalSocket>> UploaderImpl::socket() const
{
    return QFuture<shared_ptr<QLocalSocket>>();
}

QFuture<void> UploaderImpl::finish_upload()
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
