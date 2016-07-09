#include <unity/storage/qt/client/internal/remote_client/HandlerBase.h>

#include <QFuture>

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
namespace remote_client
{

HandlerBase::HandlerBase(QObject* parent,
                         QDBusPendingCall const& call,
                         function<void(QDBusPendingCallWatcher const&)> closure)
    : QObject(parent)
    , watcher_(call)
    , closure_(closure)
{
    assert(closure);
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &HandlerBase::finished);
}

void HandlerBase::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();
    closure_(*call);
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity

#include "HandlerBase.moc"
