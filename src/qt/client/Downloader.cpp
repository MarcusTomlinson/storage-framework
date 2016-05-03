#include <unity/storage/qt/client/Downloader.h>

#include <unity/storage/qt/client/internal/DownloaderImpl.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

QFuture<int> Downloader::fd() const
{
    return p_->fd();
}

QFuture<void> Downloader::close()
{
    return p_->close();
}

QFuture<void> Downloader::cancel()
{
    return p_->cancel();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
