#include <unity/storage/provider/internal/ProviderInterface.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/CredentialsCache.h>
#include <unity/storage/provider/internal/dbusmarshal.h>

#include <OnlineAccounts/AuthenticationData>
#include <QDebug>

using namespace std;

namespace unity {
namespace storage {
namespace provider {
namespace internal {

ProviderInterface::ProviderInterface(shared_ptr<ProviderBase> const& provider, shared_ptr<CredentialsCache> const& credentials, OnlineAccounts::Account* account, QObject *parent)
    : QObject(parent), provider_(provider), credentials_(credentials),
      account_(account)
{
    authenticate_account(false);
}

ProviderInterface::~ProviderInterface() = default;

void ProviderInterface::authenticate_account(bool interactive)
{
    OnlineAccounts::AuthenticationData auth_data(
        account_->authenticationMethod());
    auth_data.setInteractive(interactive);
    OnlineAccounts::PendingCall call = account_->authenticate(auth_data);
    auth_watcher_.reset(new OnlineAccounts::PendingCallWatcher(call));
    connect(auth_watcher_.get(), &OnlineAccounts::PendingCallWatcher::finished,
            this, &ProviderInterface::account_authenticated);
}

void ProviderInterface::account_authenticated()
{
    switch (account_->authenticationMethod()) {
    case OnlineAccounts::AuthenticationMethodOAuth1:
    {
        OnlineAccounts::OAuth1Reply reply(*auth_watcher_);
        if (reply.hasError())
        {
            qDebug() << "Failed to authenticate:" << reply.error().text();
        }
        else
        {
            // TODO: pass auth data to provider_.
            qDebug() << "Got OAuth1 token:" << reply.token();
        }
    }
    case OnlineAccounts::AuthenticationMethodOAuth2:
    {
        OnlineAccounts::OAuth2Reply reply(*auth_watcher_);
        if (reply.hasError())
        {
            qDebug() << "Failed to authenticate:" << reply.error().text();
        }
        else
        {
            // TODO: pass auth data to provider_.
            qDebug() << "Got OAuth2 token:" << reply.accessToken();
        }
    }
    case OnlineAccounts::AuthenticationMethodPassword:
    {
        OnlineAccounts::PasswordReply reply(*auth_watcher_);
        if (reply.hasError())
        {
            qDebug() << "Failed to authenticate:" << reply.error().text();
        }
        else
        {
            // TODO: pass auth data to provider_.
            qDebug() << "Got username:" << reply.username();
        }
    }
    default:
        qDebug() << "Unhandled authentication method:"
                 << account_->authenticationMethod();
    }
    auth_watcher_.reset();
}

void ProviderInterface::queue_request(Handler::Callback callback)
{
    unique_ptr<Handler> handler(new Handler(provider_, credentials_, callback,
                                            connection(), message()));
    connect(handler.get(), &Handler::finished, this, &ProviderInterface::request_finished);
    setDelayedReply(true);
    // If we haven't retrieved the authentication details from
    // OnlineAccounts, delay processing the handler until then.
    if (auth_watcher_)
    {
        connect(auth_watcher_.get(), &OnlineAccounts::PendingCallWatcher::finished,
                handler.get(), &Handler::begin);
    }
    else
    {
        handler->begin();
    }
    requests_.emplace(handler.get(), std::move(handler));
}

void ProviderInterface::request_finished()
{
    Handler* handler = static_cast<Handler*>(sender());
    try
    {
        auto& h = requests_.at(handler);
        h.release();
        requests_.erase(handler);
    }
    // LCOV_EXCL_START
    catch (std::out_of_range const& e)
    {
        qWarning() << "finished() called on unknown handler" << handler;
    }
    // LCOV_EXCL_STOP

    // Queue deletion of handler once we re-enter the event loop.
    handler->deleteLater();
}

QList<ProviderInterface::IMD> ProviderInterface::Roots()
{
    queue_request([](ProviderBase* provider, Context const& ctx, QDBusMessage const& message) {
            auto f = provider->roots(ctx);
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    auto roots = f.get();
                    return message.createReply(QVariant::fromValue(roots));
                });
        });
    return {};
}

QList<ProviderInterface::IMD> ProviderInterface::List(QString const& item_id, QString const& page_token, QString& /*next_token*/)
{
    queue_request([item_id, page_token](ProviderBase* provider, Context const& ctx, QDBusMessage const& message) {
            auto f = provider->list(item_id.toStdString(), page_token.toStdString(), ctx);
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    vector<Item> children;
                    string next_token;
                    tie(children, next_token) = f.get();
                    return message.createReply({
                            QVariant::fromValue(children),
                            QVariant(QString::fromStdString(next_token)),
                        });
                });
        });
    return {};
}

QList<ProviderInterface::IMD> ProviderInterface::Lookup(QString const& parent_id, QString const& name)
{
    queue_request([parent_id, name](ProviderBase* provider, Context const& ctx, QDBusMessage const& message) {
            auto f = provider->lookup(parent_id.toStdString(), name.toStdString(), ctx);
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    auto items = f.get();
                    return message.createReply(QVariant::fromValue(items));
                });
        });
    return {};
}

ProviderInterface::IMD ProviderInterface::Metadata(QString const& item_id)
{
    queue_request([item_id](ProviderBase* provider, Context const& ctx, QDBusMessage const& message) {
            auto f = provider->metadata(item_id.toStdString(), ctx);
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    auto item = f.get();
                    return message.createReply(QVariant::fromValue(item));
                });
        });
    return {};
}

ProviderInterface::IMD ProviderInterface::CreateFolder(QString const& parent_id, QString const& name)
{
    return {};
}

QString ProviderInterface::CreateFile(QString const& parent_id, QString const& name, QString const& content_type, bool allow_overwrite, QDBusUnixFileDescriptor& file_descriptor)
{
    return "";
}

QString ProviderInterface::Update(QString const& item_id, QString const& old_etag, QDBusUnixFileDescriptor& file_descriptor)
{
    return "";
}

ProviderInterface::IMD ProviderInterface::FinishUpload(QString const& upload_id)
{
    return {};
}

void ProviderInterface::CancelUpload(QString const& upload_id)
{
}

QString ProviderInterface::Download(QString const& item_id, QDBusUnixFileDescriptor& file_descriptor)
{
    return "";
}

void ProviderInterface::FinishDownload(QString const& download_id)
{
}

void ProviderInterface::Delete(QString const& item_id)
{
}

ProviderInterface::IMD ProviderInterface::Move(QString const& item_id, QString const& new_parent_id, QString const& new_name)
{
    return {};
}

ProviderInterface::IMD ProviderInterface::Copy(QString const& item_id, QString const& new_parent_id, QString const& new_name)
{
    return {};
}

}
}
}
}
