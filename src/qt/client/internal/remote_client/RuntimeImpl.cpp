#include <unity/storage/qt/client/internal/remote_client/RuntimeImpl.h>

#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>
#include <unity/storage/qt/client/internal/remote_client/dbusmarshal.h>

#include <QFutureInterface>

// TODO: Hack until we can use the registry instead
#include <OnlineAccounts/Manager>
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
        throw LocalCommsException();  // TODO, details
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
    QFutureInterface<QVector<Account::SPtr>> qf;

    if (!accounts_.isEmpty())
    {
        qf.reportResult(accounts_);
        qf.reportFinished();
        return qf.future();
    }

    OnlineAccounts::Manager manager("");
    manager.waitForReady();

    try
    {
        for (auto const& a : manager.availableAccounts())
        {
            auto impl = new AccountImpl(public_instance_, a->id(), "", a->serviceId(), a->displayName());
            Account::SPtr acc(new Account(impl));
            impl->set_public_instance(acc);
            accounts_.append(acc);
        }
        qf.reportResult(accounts_);
    }
    catch (StorageException const& e)
    {
        qf.reportException(e);
    }

    qf.reportFinished();
    return qf.future();
}

QDBusConnection& RuntimeImpl::connection()
{
    return conn_;
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "RuntimeImpl.moc"
