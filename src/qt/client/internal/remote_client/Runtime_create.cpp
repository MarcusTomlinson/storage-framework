#include <unity/storage/qt/client/Runtime.h>

#include <unity/storage/qt/client/internal/remote_client/RuntimeImpl.h>

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

Runtime::SPtr Runtime::create(QDBusConnection const& bus)
{
    auto impl = new internal::remote_client::RuntimeImpl(bus);
    Runtime::SPtr runtime(new Runtime(impl));
    impl->set_public_instance(weak_ptr<Runtime>(runtime));
    return runtime;
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
