#include <unity/storage/qt/client/internal/remote_client/ListHandler.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
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

ListHandler::ListHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                         weak_ptr<Root> const& root,
                         QString const& item_id,
                         ProviderInterface& provider,
                         QFutureInterface<QVector<shared_ptr<Item>>> qf)
    : watcher_(reply, this)
    , root_(root.lock())
    , item_id_(item_id)
    , provider_(provider)
    , qf_(qf)
{
    assert(root_);
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &ListHandler::finished);
}

QFuture<QVector<shared_ptr<Item>>> ListHandler::future()
{
    return qf_.future();
}

void ListHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<QList<storage::internal::ItemMetadata>, QString> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(ResourceException("error"));  // TODO
        qf_.reportFinished();
        return;
    }

    QVector<shared_ptr<Item>> items;
    auto metadata = reply.argumentAt<0>();
    for (auto const& md : metadata)
    {
        switch (md.type)
        {
            case ItemType::file:
            {
                auto file = FileImpl::make_file(md, root_);
                items.append(file);
                break;
            }
            case ItemType::folder:
            {
                auto folder = FolderImpl::make_folder(md, root_);
                items.append(folder);
                break;
            }
            case ItemType::root:
            {
                // TODO: log impossible item type here
                continue;
            }
            default:
            {
                abort();  // LCOV_EXCL_LINE  // Impossible
            }
        }
    }
    qf_.reportResult(items, qf_.resultCount());

    QString token = reply.argumentAt<1>();
    if (token.isEmpty())
    {
        qf_.reportFinished();  // This was the last lot of results.
    }
    else
    {
        new ListHandler(provider_.List(item_id_, token), root_, item_id_, provider_, qf_);  // Request next lot.
    }
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "ListHandler.moc"
