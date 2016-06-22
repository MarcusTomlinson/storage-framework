#include <unity/storage/qt/client/internal/remote_client/UploaderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>

#include <boost/filesystem.hpp>

#include <cassert>

#include <fcntl.h>
#include <sys/socket.h>

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

UploaderImpl::UploaderImpl(weak_ptr<File> file, ConflictPolicy policy)
    : UploaderBase(file, policy)
{
}

UploaderImpl::~UploaderImpl()
{
}

shared_ptr<File> UploaderImpl::file() const
{
    return file_;
}

shared_ptr<QLocalSocket> UploaderImpl::socket() const
{
    return nullptr;
}

QFuture<TransferState> UploaderImpl::finish_upload()
{
    return QFuture<TransferState>();
}

QFuture<void> UploaderImpl::cancel() noexcept
{
    return QFuture<void>();
}

}  // namespace remote_client
}  // namespace intternal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "UploaderImpl.moc"
