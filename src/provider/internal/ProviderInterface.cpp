#include <unity/storage/provider/internal/ProviderInterface.h>

using namespace std;

namespace unity {
namespace storage {
namespace provider {
namespace internal {

ProviderInterface::ProviderInterface(shared_ptr<ProviderBase> const& provider, QObject *parent)
    : QObject(parent), provider(provider)
{
}

ProviderInterface::~ProviderInterface() = default;

QList<ItemMetadata> ProviderInterface::Roots()
{
    return {};
}

QList<ItemMetadata> ProviderInterface::List(const QString &item_id, const QString &page_token, QString &next_token)
{
    return {};
}

QList<ItemMetadata> ProviderInterface::Lookup(const QString &parent_id, const QString &name)
{
    return {};
}

ItemMetadata ProviderInterface::Metadata(const QString &item_id)
{
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
