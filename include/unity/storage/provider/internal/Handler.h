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

class ProviderBase;
class Context;

namespace internal
{

class CredentialsCache;

class Handler : public QObject
{
    Q_OBJECT
public:
    typedef std::function<boost::future<QDBusMessage>(ProviderBase *const, Context const& ctx, QDBusMessage const&)> Callback;

    Handler(std::shared_ptr<ProviderBase> const& provider,
            std::shared_ptr<CredentialsCache> const& credentials,
            Callback const& callback,
            QDBusConnection const& bus, QDBusMessage const& message);

public Q_SLOTS:
    void begin();

Q_SIGNALS:
    void finished();

private:
    std::shared_ptr<ProviderBase> const provider_;
    std::shared_ptr<CredentialsCache> const credentials_;
    Callback const callback_;
    QDBusConnection const bus_;
    QDBusMessage const message_;

    boost::future<void> future_;
};

}
}
}
}
