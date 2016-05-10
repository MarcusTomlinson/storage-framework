#include <unity/storage/provider/internal/ProviderInterface.h>
#include <unity/storage/provider/internal/dbusmarshal.h>

#include <QDebug>

using namespace std;

namespace unity {
namespace storage {
namespace provider {
namespace internal {

ProviderInterface::ProviderInterface(shared_ptr<ProviderBase> const& provider, QObject *parent)
    : QObject(parent), provider_(provider)
{
}

ProviderInterface::~ProviderInterface() = default;

void ProviderInterface::queueRequest(Handler::Callback callback)
{
    unique_ptr<Handler> handler(new Handler(provider_, callback,
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
    queueRequest([](ProviderBase* provider, QDBusMessage const& message) {
            auto f = provider->roots();
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    auto roots = f.get();
                    return message.createReply(QVariant::fromValue(roots));
                });
        });
    return {};
}

QList<ItemMetadata> ProviderInterface::List(const QString &item_id, const QString &page_token, QString &next_token)
{
    queueRequest([item_id, page_token](ProviderBase* provider, QDBusMessage const& message) {
            auto f = provider->list(item_id.toStdString(), page_token.toStdString());
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

QList<ItemMetadata> ProviderInterface::Lookup(const QString &parent_id, const QString &name)
{
    queueRequest([parent_id, name](ProviderBase* provider, QDBusMessage const& message) {
            auto f = provider->lookup(parent_id.toStdString(), name.toStdString());
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    auto items = f.get();
                    return message.createReply(QVariant::fromValue(items));
                });
        });
    return {};
}

ItemMetadata ProviderInterface::Metadata(const QString &item_id)
{
    queueRequest([item_id](ProviderBase* provider, QDBusMessage const& message) {
            auto f = provider->metadata(item_id.toStdString());
            return f.then([=](decltype(f) f) -> QDBusMessage {
                    auto item = f.get();
                    return message.createReply(QVariant::fromValue(item));
                });
        });
    return {};
}

ItemMetadata ProviderInterface::CreateFolder(const QString &parent_id, const QString &name)
{
    return {};
}

QString ProviderInterface::CreateFile(const QString &parent_id, const QString &name, const QString &content_type, bool allow_overwrite, QDBusUnixFileDescriptor &upload_fd)
{
    return "";
}

QString ProviderInterface::Update(const QString &item_id, const QString &old_etag, QDBusUnixFileDescriptor &upload_fd)
{
    return "";
}

ItemMetadata ProviderInterface::FinishUpload(const QString &upload_id)
{
    return {};
}

void ProviderInterface::CancelUpload(const QString &upload_id)
{
}

QString ProviderInterface::Download(const QString &item_id, QDBusUnixFileDescriptor &download_fd)
{
    return "";
}

void ProviderInterface::FinishDownload(const QString &download_id)
{
}

void ProviderInterface::Delete(const QString &item_id)
{
}

ItemMetadata ProviderInterface::Move(const QString &item_id, const QString &new_parent_id, const QString &new_name)
{
    return {};
}

ItemMetadata ProviderInterface::Copy(const QString &item_id, const QString &new_parent_id, const QString &new_name)
{
    return {};
}

}
}
}
}
