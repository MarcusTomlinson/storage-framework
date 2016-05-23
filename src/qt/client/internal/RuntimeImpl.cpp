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

QFuture<QVector<shared_ptr<Account>>> RuntimeImpl::get_accounts()
{
    return QFuture<QVector<shared_ptr<Account>>>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
