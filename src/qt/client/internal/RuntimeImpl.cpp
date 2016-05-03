#include <unity/storage/qt/client/internal/RuntimeImpl.h>

#include <cassert>

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

QFuture<QVector<Account::UPtr>> RuntimeImpl::get_accounts()
{
    return QFuture<QVector<Account::UPtr>>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
