#include <unity/storage/provider/internal/DisconnectWatcher.h>

#include <cassert>

namespace unity
{

namespace storage
{

namespace provider
{

namespace internal
{

DisconnectWatcher::DisconnectWatcher(QDBusConnection const& bus, QObject *parent)
    : QObject(parent)
{
    watcher_.setConnection(bus);
    watcher_.setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(&watcher_, &QDBusServiceWatcher::serviceUnregistered,
            this, &DisconnectWatcher::serviceDisconnected,
            Qt::DirectConnection);
}

DisconnectWatcher::~DisconnectWatcher() = default;

void DisconnectWatcher::watch(QString const& peer)
{
    assert(peer[0] == ':');
    auto it = services_.find(peer);
    if (it != services_.end())
    {
        it->second++;
    }
    else
    {
        watcher_.addWatchedService(peer);
        services_[peer] = 1;
    }
}

void DisconnectWatcher::unwatch(QString const& peer)
{
    assert(peer[0] == ':');
    auto it = services_.find(peer);
    if (it == services_.end())
    {
        return;
    }
    it->second--;
    if (it->second == 0)
    {
        services_.erase(it);
        watcher_.removeWatchedService(peer);
    }
}

}
}
}
}
