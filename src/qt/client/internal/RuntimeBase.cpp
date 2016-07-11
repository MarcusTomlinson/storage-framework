#include <unity/storage/qt/client/internal/RuntimeBase.h>

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

RuntimeBase::RuntimeBase()
    : destroyed_(false)
{
}

void RuntimeBase::set_public_instance(weak_ptr<Runtime> p)
{
    assert(p.lock());
    public_instance_ = p;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
