#include <unity/storage/qt/client/internal/remote_client/DownloaderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/StorageSocket.h>

#include <cassert>

#include <sys/socket.h>

using namespace unity::storage::qt::client;
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
namespace remote_client
{

DownloaderImpl::DownloaderImpl(weak_ptr<File> file)
    : DownloaderBase(file.lock())
{
}

DownloaderImpl::~DownloaderImpl()
{
}

shared_ptr<File> DownloaderImpl::file() const
{
    return file_;
}

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

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "DownloaderImpl.moc"
