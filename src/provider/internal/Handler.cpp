#include <unity/storage/provider/internal/Handler.h>
#include <unity/storage/provider/ProviderBase.h>

#include <stdexcept>

using namespace std;

namespace
{
char const ERROR[] = "com.canonical.StorageFramework.Provider.Error";
}

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

Handler::Handler(shared_ptr<ProviderBase> const& provider,
                 Callback const& callback,
                 QDBusConnection const& bus, QDBusMessage const& message)
    : provider_(provider), callback_(callback), bus_(bus), message_(message)
{
}

void Handler::begin()
{
    // Need to put security check in here.
    auto f = callback_(provider_.get(), message_);
    future_ = f.then([this](boost::future<QDBusMessage> f) {
            try
            {
                reply_ = f.get();
            }
            catch (std::exception const& e)
            {
                reply_ = message_.createErrorReply(ERROR, e.what());
            }
            // queue the call to send_reply so it happens in the event
            // loop thread.
            QMetaObject::invokeMethod(this, "send_reply", Qt::QueuedConnection);
        });
}

void Handler::send_reply()
{
    bus_.send(reply_);
    Q_EMIT finished();
}

}
}
}
}
