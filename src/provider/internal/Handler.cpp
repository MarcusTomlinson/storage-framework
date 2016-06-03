#include <unity/storage/provider/internal/Handler.h>
#include <unity/storage/provider/internal/CredentialsCache.h>
#include <unity/storage/provider/internal/MainLoopExecutor.h>
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
    boost::future<QDBusMessage> msg_future = cred_future.then(
        //MainLoopExecutor::instance(),
        [this](decltype(cred_future) f) -> boost::future<QDBusMessage> {
            auto creds = f.get();
            if (!creds.valid) {
                throw std::runtime_error("Handler::begin(): could not retrieve credentials");
            }
            Context ctx{creds.uid, creds.pid, std::move(creds.label)};
            return callback_(provider_.get(), ctx, message_);
        });
    future_ = msg_future.then(
        MainLoopExecutor::instance(),
        [this](decltype(msg_future) f) {
            QDBusMessage reply;
            try
            {
                reply = f.get();
            }
            catch (std::exception const& e)
            {
                reply = message_.createErrorReply(ERROR, e.what());
            }
            bus_.send(reply);
            Q_EMIT finished();
        });
}

}
}
}
}
