#include <unity/storage/qt/client/internal/AccountBase.h>

#include <unity/storage/qt/client/Exceptions.h>

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

AccountBase::AccountBase(weak_ptr<Runtime> const& runtime)
    : runtime_(runtime)
{
    assert(runtime.lock());
}

Runtime* AccountBase::runtime() const
{
    if (auto runtime = runtime_.lock())
    {
        return runtime.get();
    }
    throw RuntimeDestroyedException();  // TODO
}

void AccountBase::set_public_instance(weak_ptr<Account> const& p)
{
    public_instance_ = p;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
