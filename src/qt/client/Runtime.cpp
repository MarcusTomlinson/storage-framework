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

Runtime::SPtr Runtime::create()
{
    auto impl = new internal::RuntimeImpl;
    Runtime::SPtr runtime(new Runtime(impl));
    impl->set_public_instance(weak_ptr<Runtime>(runtime));
    return runtime;
}

QFuture<QVector<shared_ptr<Account>>> Runtime::accounts()
{
    return p_->get_accounts();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
