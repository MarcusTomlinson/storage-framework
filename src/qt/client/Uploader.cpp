#include <unity/storage/qt/client/Uploader.h>

#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/UploaderBase.h>

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

Uploader::Uploader(internal::UploaderBase* p)
    : p_(p)
{
}

Uploader::~Uploader() = default;

std::shared_ptr<QLocalSocket> Uploader::socket() const
{
    return p_->socket();
}

QFuture<shared_ptr<File>> Uploader::finish_upload()
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
