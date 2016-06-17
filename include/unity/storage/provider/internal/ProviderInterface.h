#pragma once

#include <unity/storage/provider/internal/Handler.h>

#include <QObject>
#include <QList>
#include <QDBusConnection>
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

class AccountData;

struct ItemMetadata
{
};

class ProviderInterface : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    ProviderInterface(std::shared_ptr<AccountData> const& account_data,
                      QObject *parent=nullptr);
    ~ProviderInterface();

public Q_SLOTS:

    QList<ItemMetadata> Roots();
    QList<ItemMetadata> List(QString const& item_id, QString const& page_token, QString& next_token);
    QList<ItemMetadata> Lookup(QString const& parent_id, QString const& name);
    ItemMetadata Metadata(QString const& item_id);
    ItemMetadata CreateFolder(QString const& parent_id, QString const& name);
    QString CreateFile(QString const& parent_id, QString const& name, QString const& content_type, bool allow_overwrite, QDBusUnixFileDescriptor& file_descriptor);
    QString Update(QString const& item_id, QString const& old_etag, QDBusUnixFileDescriptor& file_descriptor);
    ItemMetadata FinishUpload(QString const& upload_id);
    void CancelUpload(QString const& upload_id);
    QString Download(QString const& item_id, QDBusUnixFileDescriptor& file_descriptor);
    void FinishDownload(QString const& download_id);
    void Delete(QString const& item_id);
    ItemMetadata Move(QString const& item_id, QString const& new_parent_id, QString const& new_name);
    ItemMetadata Copy(QString const& item_id, QString const& new_parent_id, QString const& new_name);

private Q_SLOTS:
    void request_finished();

private:
    void queue_request(Handler::Callback callback);

    std::shared_ptr<AccountData> const account_;
    std::map<Handler*, std::unique_ptr<Handler>> requests_;

    Q_DISABLE_COPY(ProviderInterface)
};

}
}
}
}
