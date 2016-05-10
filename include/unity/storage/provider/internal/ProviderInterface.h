#pragma once

#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/Handler.h>

#include <QObject>
#include <QList>
#include <QDBusContext>
#include <QDBusUnixFileDescriptor>

#include <map>
#include <memory>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

struct ItemMetadata
{
};

class ProviderInterface : public QObject, private QDBusContext
{
    Q_OBJECT

public:
    ProviderInterface(std::shared_ptr<ProviderBase> const& provider, QObject *parent=nullptr);
    ~ProviderInterface();

public Q_SLOTS:

    QList<ItemMetadata> Roots();
    QList<ItemMetadata> List(const QString &item_id, const QString &page_token, QString &next_token);
    QList<ItemMetadata> Lookup(const QString &parent_id, const QString &name);
    ItemMetadata Metadata(const QString &item_id);
    ItemMetadata CreateFolder(const QString &parent_id, const QString &name);
    QString CreateFile(const QString &parent_id, const QString &name, const QString &content_type, bool allow_overwrite, QDBusUnixFileDescriptor &upload_fd);
    QString Update(const QString &item_id, const QString &old_etag, QDBusUnixFileDescriptor &upload_fd);
    ItemMetadata FinishUpload(const QString &upload_id);
    void CancelUpload(const QString &upload_id);
    QString Download(const QString &item_id, QDBusUnixFileDescriptor &download_fd);
    void FinishDownload(const QString &download_id);
    void Delete(const QString &item_id);
    ItemMetadata Move(const QString &item_id, const QString &new_parent_id, const QString &new_name);
    ItemMetadata Copy(const QString &item_id, const QString &new_parent_id, const QString &new_name);

private Q_SLOTS:
    void requestFinished();

private:
    void queueRequest(Handler::Callback callback);

    std::shared_ptr<ProviderBase> provider_;
    std::map<Handler*, std::unique_ptr<Handler>> requests_;
};

}
}
}
}
