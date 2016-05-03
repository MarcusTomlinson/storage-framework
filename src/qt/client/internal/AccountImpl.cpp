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

QFuture<QVector<Root::UPtr>> AccountImpl::get_roots() const
{
    return QFuture<QVector<Root::UPtr>>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
