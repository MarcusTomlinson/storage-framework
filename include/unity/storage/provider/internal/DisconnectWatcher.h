#pragma once

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QObject>
#include <QString>

#include <map>

namespace unity
{

namespace storage
{

namespace provider
{

namespace internal
{


class DisconnectWatcher : public QObject {
    Q_OBJECT
public:

    explicit DisconnectWatcher(QDBusConnection const& bus, QObject *parent=nullptr);
    virtual ~DisconnectWatcher();

    void watch(QString const& peer);
    void unwatch(QString const& peer);

Q_SIGNALS:
    void serviceDisconnected(QString const& serviceName);

private:
    QDBusServiceWatcher watcher_;
    std::map<QString,int> services_;
};

}  // namespace internal

}  // namespace provider

}  // namespace storage

}  // namespace unity
