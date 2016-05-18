#include <unity/storage/provider/internal/Handler.h>
#include <unity/storage/provider/internal/CredentialsCache.h>
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
                 shared_ptr<CredentialsCache> const& credentials,
                 Callback const& callback,
                 QDBusConnection const& bus, QDBusMessage const& message)
    : provider_(provider), credentials_(credentials),
      callback_(callback), bus_(bus), message_(message)
{
}

void Handler::begin()
{
    // Need to put security check in here.
    auto cred_future = credentials_->get(message_.service());
    boost::future<QDBusMessage> msg_future = cred_future.then([this](decltype(cred_future) f) -> boost::future<QDBusMessage> {
            auto creds = f.get();
            if (!creds.valid) {
                throw std::runtime_error("Handler::begin(): could not retrieve credentials");
            }
            Context ctx{creds.uid, creds.pid, std::move(creds.label)};
            return callback_(provider_.get(), ctx, message_);
        });
    future_ = msg_future.then([this](decltype(msg_future) f) {
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
