#include <unity/storage/qt/client/internal/RuntimeImpl.h>

#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/internal/AccountImpl.h>

#include <QAbstractSocket>
#include <QFutureInterface>

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

RuntimeImpl::RuntimeImpl()
    : destroyed_(false)
{
    qRegisterMetaType<QAbstractSocket::SocketState>();  // Qt doesn't do this for us.
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
    if (destroyed_)
    {
        return;
    }
    destroyed_ = true;
}

QFuture<QVector<Account::SPtr>> RuntimeImpl::accounts()
{

    char const* user = g_get_user_name();
    assert(*user != '\0');
    QString owner = user;

    QString owner_id;
    owner_id.setNum(getuid());

    QString description = "Account for " + owner + " (" + owner_id + ")";

    QFutureInterface<QVector<Account::SPtr>> qf;

    if (!accounts_.isEmpty())
    {
        qf.reportResult(accounts_);
        qf.reportFinished();
        return qf.future();
    }

    // Create accounts_ on first access.
    auto impl = new AccountImpl(owner, owner_id, description);
    Account::SPtr acc(new Account(impl));
    impl->set_public_instance(weak_ptr<Account>(acc));
    impl->set_runtime(public_instance_);
    accounts_.append(acc);
    qf.reportResult(accounts_);
    qf.reportFinished();
    return qf.future();
}

void RuntimeImpl::set_public_instance(weak_ptr<Runtime> p)
{
    public_instance_ = p;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
