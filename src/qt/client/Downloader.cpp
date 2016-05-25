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

Downloader::Downloader(internal::DownloaderImpl* p)
    : p_(p)
{
}

Downloader::~Downloader() = default;

std::shared_ptr<File> Downloader::file() const
{
    return p_->file();
}

std::shared_ptr<StorageSocket> Downloader::socket() const
{
    return p_->socket();
}

QFuture<TransferState> Downloader::finish_download()
{
    return p_->finish_download();
}

QFuture<void> Downloader::cancel()
{
    return p_->cancel();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
