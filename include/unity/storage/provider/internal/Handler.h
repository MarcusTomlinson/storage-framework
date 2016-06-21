#pragma once

#include <boost/thread/future.hpp>

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>

#include <functional>
#include <memory>

namespace unity
{
namespace storage
{
namespace provider
{

class Context;
class ProviderBase;

namespace internal
{

class AccountData;
class PendingJobs;

class Handler : public QObject
{
    Q_OBJECT
public:
    typedef std::function<boost::future<QDBusMessage>(std::shared_ptr<AccountData> const&, Context const&, QDBusMessage const&)> Callback;

    Handler(std::shared_ptr<AccountData> const& account,
            Callback const& callback,
            QDBusConnection const& bus, QDBusMessage const& message);

public Q_SLOTS:
    void begin();

Q_SIGNALS:
    void finished();

private:
    std::shared_ptr<AccountData> const account_;
    Callback const callback_;
    QDBusConnection const bus_;
    QDBusMessage const message_;

    boost::future<void> future_;

    Q_DISABLE_COPY(Handler)
};

}
}
}
}
