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

Uploader::Uploader(internal::UploaderImpl* p)
    : p_(p)
{
}

Uploader::~Uploader() = default;

std::shared_ptr<File> Uploader::file() const
{
    return p_->file();
}

std::shared_ptr<StorageSocket> Uploader::socket() const
{
    return p_->socket();
}

QFuture<TransferState> Uploader::finish_upload()
{
    return p_->finish_upload();
}

QFuture<void> Uploader::cancel()
{
    return p_->cancel();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
