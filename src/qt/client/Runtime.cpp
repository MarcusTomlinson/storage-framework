#include <unity/storage/qt/client/Runtime.h>

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

Runtime::Runtime(internal::RuntimeImpl* p)
    : p_(p)
{
    assert(p != nullptr);
}

Runtime::UPtr Runtime::create()
{
    return Runtime::UPtr(new Runtime(new internal::RuntimeImpl));
}

QFuture<QVector<Account::UPtr>> Runtime::get_accounts()
{
    return p_->get_accounts();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
