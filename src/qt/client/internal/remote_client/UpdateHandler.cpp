#include <unity/storage/qt/client/internal/remote_client/UpdateHandler.h>

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

UpdateHandler::UpdateHandler(QDBusPendingReply<QString, int> const& reply,
                             QString const& old_etag,
                             weak_ptr<Root> root,
                             ProviderInterface& provider)
    : old_etag_(old_etag)
    , root_(root.lock())
    , provider_(provider)
    , watcher_(reply, this)
{
    assert(root_);
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &UpdateHandler::finished);
    qf_.reportStarted();
}

QFuture<shared_ptr<Uploader>> UpdateHandler::future()
{
    return qf_.future();
}

void UpdateHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<QString, int> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    auto upload_id = reply.argumentAt<0>();
    auto fd = reply.argumentAt<1>();
    if (fd < 0)
    {
        qf_.reportException(StorageException());  // TODO
    }
    else
    {
        auto uploader = UploaderImpl::make_uploader(upload_id, fd, old_etag_, root_, provider_);
        qf_.reportResult(uploader);
    }

    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "UpdateHandler.moc"
