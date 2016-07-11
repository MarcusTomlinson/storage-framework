#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <QDBusPendingCallWatcher>
#pragma GCC diagnostic pop
#include <QObject>

#include <functional>

class QDBusPendingCall;

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

class HandlerBase : public QObject
{
    Q_OBJECT

public:
    HandlerBase(QObject* parent,
                QDBusPendingCall const& call,
                std::function<void(QDBusPendingCallWatcher const&)> closure);

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

protected:
    QDBusPendingCallWatcher watcher_;

private:
    std::function<void(QDBusPendingCallWatcher const&)> closure_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
