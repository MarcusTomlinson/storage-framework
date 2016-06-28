#include <unity/storage/qt/client/internal/remote_client/RootsHandler.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/dbusmarshal.h>
#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>

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

RootsHandler::RootsHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                           weak_ptr<Account> const& account)
    : watcher_(reply, this)
    , account_(account.lock())
{
    assert(account_);
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &RootsHandler::finished);
    qf_.reportStarted();
}

QFuture<QVector<shared_ptr<Root>>> RootsHandler::future()
{
    return qf_.future();
}

void RootsHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<QList<storage::internal::ItemMetadata>> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
    }
    else
    {
        QVector<shared_ptr<Root>> roots;
        auto metadata = reply.value();
        for (auto const& md : metadata)
        {
            if (md.type != ItemType::root)
            {
                // TODO: log impossible item type here
                continue;
            }
            auto root = RootImpl::make_root(md, account_);
            roots.append(root);
        }
        qf_.reportResult(roots);
    }
    qf_.reportFinished();
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "RootsHandler.moc"
