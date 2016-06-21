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

void RuntimeImpl::shutdown()
{
    if (destroyed_)
    {
        return;
    }
    destroyed_ = true;
}

QFuture<QVector<shared_ptr<Account>>> RuntimeImpl::accounts()
{
    return QFuture<QVector<shared_ptr<Account>>>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
