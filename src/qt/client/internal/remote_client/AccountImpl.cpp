#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/Runtime.h>
#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>
#include <unity/storage/qt/client/internal/remote_client/RuntimeImpl.h>

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

static constexpr char BUS_NAME[] = "com.canonical.StorageFramework.Provider.ProviderTest";

AccountImpl::AccountImpl(weak_ptr<Runtime> const& runtime,
                         int account_id,
                         QString const& owner,
                         QString const& owner_id,
                         QString const& description)
    : AccountBase(runtime)
    , owner_(owner)
    , owner_id_(owner_id)
    , description_(description)
{
    auto rt_impl = dynamic_pointer_cast<RuntimeImpl>(runtime.lock()->p_);
    assert(rt_impl);
    QString bus_path = "/provider/" + QString::number(account_id);
    provider_.reset(new ProviderInterface(BUS_NAME, bus_path, rt_impl->connection()));
    if (!provider_->isValid())
    {
        throw LocalCommsException();  // TODO, details
    }
}

QString AccountImpl::owner() const
{
    return owner_;
}

QString AccountImpl::owner_id() const
{
    return owner_id_;
}

QString AccountImpl::description() const
{
    return description_;
}

// TODO: Move to header file?

namespace
{

class RootsHandler : public QObject
{
    Q_OBJECT

public:
    RootsHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                 weak_ptr<Account> const& account);

    QFuture<QVector<Root::SPtr>> future()
    {
        return qf_.future();
    }

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<QVector<Root::SPtr>> qf_;
    weak_ptr<Account> account_;
};

RootsHandler::RootsHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                           weak_ptr<Account> const& account)
    : watcher_(reply, this)
    , account_(account)
{
    assert(account.lock());
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &RootsHandler::finished);
    qf_.reportStarted();
}

void RootsHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<QList<storage::internal::ItemMetadata>> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();
        qf_.reportException(StorageException());  // TODO
    }
    else
    {
        QVector<Root::SPtr> roots;
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

}  // namespace

QFuture<QVector<Root::SPtr>> AccountImpl::roots()
{
    auto handler = new RootsHandler(provider_->Roots(), public_instance_);
    return handler->future();
}

ProviderInterface& AccountImpl::provider()
{
    return *provider_;
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "AccountImpl.moc"
