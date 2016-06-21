#include <unity/storage/provider/internal/Handler.h>
#include <unity/storage/provider/internal/AccountData.h>
#include <unity/storage/provider/internal/DBusPeerCache.h>
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

Handler::Handler(shared_ptr<AccountData> const& account,
                 Callback const& callback,
                 QDBusConnection const& bus, QDBusMessage const& message)
    : account_(account), callback_(callback), bus_(bus), message_(message)
{
}

void Handler::begin()
{
    // Need to put security check in here.
    auto peer_future = account_->dbus_peer().get(message_.service());
    boost::future<QDBusMessage> msg_future = peer_future.then(
        //MainLoopExecutor::instance(),
        [this](decltype(peer_future) f) -> boost::future<QDBusMessage> {
            auto info = f.get();
            if (!info.valid) {
                throw std::runtime_error("Handler::begin(): could not retrieve credentials");
            }
            Context ctx{info.uid, info.pid, std::move(info.label), account_->credentials()};
            return callback_(account_, ctx, message_);
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
