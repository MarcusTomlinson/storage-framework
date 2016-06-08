#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>

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

AccountImpl::AccountImpl(weak_ptr<Runtime> const& runtime,
                         QString const& owner,
                         QString const& owner_id,
                         QString const& description)
    : AccountBase(runtime)
{
}

QString AccountImpl::owner() const
{
    return "";
}

QString AccountImpl::owner_id() const
{
    return "";
}

QString AccountImpl::description() const
{
    return "";
}

QFuture<QVector<Root::SPtr>> AccountImpl::roots()
{
    using namespace boost::filesystem;

    QFutureInterface<QVector<Root::SPtr>> qf;
    qf.reportFinished();
    return qf.future();
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
