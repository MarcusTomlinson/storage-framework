#include <unity/storage/qt/client/internal/remote_client/CreateFileHandler.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/UploaderImpl.h>

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
namespace remote_client
{

CreateFileHandler::CreateFileHandler(QDBusPendingReply<QString, QDBusUnixFileDescriptor> const& reply,
                                     int64_t size,
                                     weak_ptr<Root> const& root,
                                     ProviderInterface& provider)
    : watcher_(reply, this)
    , size_(size)
    , root_(root.lock())
    , provider_(provider)
{
    assert(size >= 0);
    assert(root_);
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &CreateFileHandler::finished);
    qf_.reportStarted();
}

QFuture<shared_ptr<Uploader>> CreateFileHandler::future()
{
    return qf_.future();
}

void CreateFileHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<QString, QDBusUnixFileDescriptor> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    auto upload_id = reply.argumentAt<0>();
    auto fd = reply.argumentAt<1>();
    auto uploader = UploaderImpl::make_uploader(upload_id, fd.fileDescriptor(), size_, "", root_, provider_);
    qf_.reportResult(uploader);
    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "CreateFileHandler.moc"
