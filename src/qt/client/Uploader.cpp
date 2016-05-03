#include <unity/storage/qt/client/Uploader.h>

#include <unity/storage/qt/client/internal/UploaderImpl.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

QFuture<int> Uploader::fd() const
{
    return p_->fd();
}

QFuture<void> Uploader::close()
{
    return p_->close();
}

QFuture<void> Uploader::cancel()
{
    return p_->cancel();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
