#include <unity/storage/qt/client/internal/local_client/RuntimeImpl.h>

#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/internal/make_future.h>
#include <unity/storage/qt/client/internal/local_client/AccountImpl.h>

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
namespace local_client
{

RuntimeImpl::RuntimeImpl()
{
    qRegisterMetaType<QAbstractSocket::SocketState>();
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

    char const* user = g_get_user_name();
    assert(*user != '\0');
    QString owner = user;

    QString owner_id;
    owner_id.setNum(getuid());

    QString description = "Account for " + owner + " (" + owner_id + ")";

    QFutureInterface<QVector<Account::SPtr>> qf;

    if (!accounts_.isEmpty())
    {
        return make_ready_future(accounts_);
    }

    // Create accounts_ on first access.
    auto impl = new AccountImpl(public_instance_, owner, owner_id, description);
    Account::SPtr acc(new Account(impl));
    impl->set_public_instance(acc);
    accounts_.append(acc);
    return make_ready_future(accounts_);
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
