#include <unity/storage/qt/client/Runtime.h>

#include <unity/storage/qt/client/internal/RuntimeBase.h>

#include <QDBusConnection>

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

// Runtime::SPtr Runtime::create() is defined by local_client and remote_client, respectively.

Runtime::Runtime(internal::RuntimeBase* p)
    : p_(p)
{
    assert(p != nullptr);
}

Runtime::~Runtime()
{
    shutdown();
}

Runtime::SPtr Runtime::create()
{
    return Runtime::create(QDBusConnection::sessionBus());
}

void Runtime::shutdown()
{
    p_->shutdown();

}

QFuture<QVector<shared_ptr<Account>>> Runtime::accounts()
{
    return p_->accounts();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
