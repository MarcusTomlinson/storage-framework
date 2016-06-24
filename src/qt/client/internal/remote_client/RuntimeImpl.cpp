#include <unity/storage/qt/client/internal/remote_client/RuntimeImpl.h>

#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>
#include <unity/storage/qt/client/internal/remote_client/dbusmarshal.h>

#include <QFutureInterface>
#include <QDBusMetaType>

// TODO: Hack until we can use the registry instead
#include <OnlineAccounts/Account>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <glib.h>
#pragma GCC diagnostic pop

#include <cassert>
#include <cstdlib>

#include <unistd.h>

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

RuntimeImpl::RuntimeImpl()
    : conn_(QDBusConnection::sessionBus())
{
    if (!conn_.isConnected())
    {
        qDebug() << "can't connect to session bus";
        throw LocalCommsException();  // LCOV_EXCL_LINE  // TODO, details
    }
    qDBusRegisterMetaType<unity::storage::internal::ItemMetadata>();
    qDBusRegisterMetaType<QList<unity::storage::internal::ItemMetadata>>();
}

RuntimeImpl::~RuntimeImpl()
{
    try
    {
        shutdown();
    }
    catch (std::exception const&)
    {
    }
}

void RuntimeImpl::shutdown()
{
    if (destroyed_.exchange(true))
    {
        return;
    }
}

QFuture<QVector<Account::SPtr>> RuntimeImpl::accounts()
{
    if (!manager_)
    {
        manager_.reset(new OnlineAccounts::Manager(""));
        connect(manager_.get(), &OnlineAccounts::Manager::ready, this, &RuntimeImpl::manager_ready);
        connect(&timer_, &QTimer::timeout, this, &RuntimeImpl::timeout);
        timer_.setSingleShot(true);
        timer_.start(5000);  // TODO: should be configurable?
    }

    qf_.reportStarted();
    return qf_.future();
}

QDBusConnection& RuntimeImpl::connection()
{
    return conn_;
}

void RuntimeImpl::manager_ready()
{
    timer_.stop();
    try
    {
        QVector<Account::SPtr> accounts;
        for (auto const& a : manager_->availableAccounts("google-drive-scope"))
        {
            qDebug() << "got account:" << a->displayName() << a->serviceId() << a->id();
            auto impl = new AccountImpl(public_instance_, a->id(), "", a->serviceId(), a->displayName());
            Account::SPtr acc(new Account(impl));
            impl->set_public_instance(acc);
            accounts.append(acc);
        }
        qf_.reportResult(accounts);
    }
    catch (StorageException const& e)
    {
        qf_.reportException(e);
    }
    qf_.reportFinished();
}

void RuntimeImpl::timeout()
{
    qf_.reportException(StorageException());  // TODO
    qf_.reportFinished();
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "RuntimeImpl.moc"
