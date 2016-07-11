#pragma once

#include <QDBusPendingCallWatcher>
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
