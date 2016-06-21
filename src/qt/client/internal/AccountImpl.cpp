#include <unity/storage/qt/client/internal/AccountImpl.h>

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

QFuture<QVector<shared_ptr<Root>>> AccountImpl::roots()
{
    return QFuture<QVector<shared_ptr<Root>>>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
