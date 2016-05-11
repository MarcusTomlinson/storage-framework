#include <unity/storage/qt/client/internal/RuntimeImpl.h>

#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/internal/AccountImpl.h>

#include <QFutureInterface>

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

QFuture<QVector<Account::SPtr>> RuntimeImpl::get_accounts()
{

    char* user = getlogin();
    assert(user && *user != '\0');
    QString owner = user;

    QString owner_id;
    owner_id.setNum(getuid());

    QString description = "Account for " + owner + " (" + owner_id + ")";

    auto impl = new AccountImpl(owner, owner_id, description);
    Account::SPtr acc(new Account(impl));
    impl->set_public_instance(weak_ptr<Account>(acc));
    impl->set_runtime(public_instance_);  // TODO, this is broken

    QVector<Account::SPtr> accounts;
    accounts.append(acc);

    QFutureInterface<QVector<Account::SPtr>> qf;
    qf.reportResult(accounts);
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
