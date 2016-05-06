#define BOOST_THREAD_VERSION 4
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

namespace internal
{

class Handler : public QObject
{
    Q_OBJECT
public:
    typedef std::function<boost::future<QDBusMessage>(ProviderBase *const, QDBusConnection const&, QDBusMessage const&)> Callback;

    Handler(std::shared_ptr<ProviderBase> const& provider,
            Callback const& callback,
            QDBusConnection const& bus, QDBusMessage const& message);

private Q_SLOTS:
    void send_reply();

Q_SIGNALS:
    void finished();

private:
    std::shared_ptr<ProviderBase> const provider_;
    Callback const callback_;
    QDBusConnection const bus_;
    QDBusMessage const message_;

    boost::future<void> future_;
    QDBusMessage reply_;
};

}
}
}
}
