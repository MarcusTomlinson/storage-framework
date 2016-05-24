#include <unity/storage/provider/internal/ProviderInterface.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/CredentialsCache.h>
#include <unity/storage/provider/internal/dbusmarshal.h>

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
}

ProviderInterface::~ProviderInterface() = default;

void ProviderInterface::queueRequest(Handler::Callback callback)
{
    unique_ptr<Handler> handler(new Handler(provider_, credentials_, callback,
                                            connection(), message()));
    connect(handler.get(), &Handler::finished, this, &ProviderInterface::requestFinished);
    setDelayedReply(true);
    handler->begin();
    requests_.emplace(handler.get(), std::move(handler));
}

void ProviderInterface::requestFinished()
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

QList<ItemMetadata> ProviderInterface::Roots()
{
    queueRequest([](ProviderBase* provider, Context const& ctx, QDBusMessage const& message) {
            auto f = provider->roots(ctx);
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    auto roots = f.get();
                    return message.createReply(QVariant::fromValue(roots));
                });
        });
    return {};
}

QList<ItemMetadata> ProviderInterface::List(QString const& item_id, QString const& page_token, QString& /*next_token*/)
{
    queueRequest([item_id, page_token](ProviderBase* provider, Context const& ctx, QDBusMessage const& message) {
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

QList<ItemMetadata> ProviderInterface::Lookup(QString const& parent_id, QString const& name)
{
    queueRequest([parent_id, name](ProviderBase* provider, Context const& ctx, QDBusMessage const& message) {
            auto f = provider->lookup(parent_id.toStdString(), name.toStdString(), ctx);
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    auto items = f.get();
                    return message.createReply(QVariant::fromValue(items));
                });
        });
    return {};
}

ItemMetadata ProviderInterface::Metadata(QString const& item_id)
{
    queueRequest([item_id](ProviderBase* provider, Context const& ctx, QDBusMessage const& message) {
            auto f = provider->metadata(item_id.toStdString(), ctx);
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    auto item = f.get();
                    return message.createReply(QVariant::fromValue(item));
                });
        });
    return {};
}

ItemMetadata ProviderInterface::CreateFolder(QString const& parent_id, QString const& name)
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

ItemMetadata ProviderInterface::FinishUpload(QString const& upload_id)
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

ItemMetadata ProviderInterface::Move(QString const& item_id, QString const& new_parent_id, QString const& new_name)
{
    return {};
}

ItemMetadata ProviderInterface::Copy(QString const& item_id, QString const& new_parent_id, QString const& new_name)
{
    return {};
}

}
}
}
}
