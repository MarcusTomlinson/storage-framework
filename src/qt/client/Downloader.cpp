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

std::shared_ptr<File> Downloader::file() const
{
    return p_->file();
}

QFuture<std::shared_ptr<QLocalSocket>> Downloader::socket() const
{
    return p_->socket();
}

QFuture<void> Downloader::finish_download()
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
