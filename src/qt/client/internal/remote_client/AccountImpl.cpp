#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>

#include <unity/storage/internal/ItemMetadata.h>
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
    RootsHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply);

    QFuture<QVector<Root::SPtr>> future();

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<QVector<Root::SPtr>> qf_;
};

RootsHandler::RootsHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply)
    : watcher_(reply, this)
{
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &RootsHandler::finished);
    qf_.reportStarted();
}

QFuture<QVector<Root::SPtr>> RootsHandler::future()
{
    return qf_.future();
}

void RootsHandler::finished(QDBusPendingCallWatcher* call)
{
    QDBusPendingReply<QList<storage::internal::ItemMetadata>> reply = *call;
    this->deleteLater();
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
            //auto root = RootImpl::make_root(md, dynamic_pointer_cast<Root>(public_instance_));
            //roots.append(root);
        }
        qf_.reportResult(roots);
    }
    qf_.reportFinished();
}

}  // namespace

QFuture<QVector<Root::SPtr>> AccountImpl::roots()
{
    qDebug() << "creating handler";
    auto handler = new RootsHandler(provider_->Roots());  // Deletes itself later
    qDebug() << "returning future";
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
