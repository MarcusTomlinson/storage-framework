#pragma once

#include <unity/storage/qt/client/internal/DownloaderBase.h>

#include <QDBusUnixFileDescriptor>

class ProviderInterface;
class QLocalSocket;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Downloader;

namespace internal
{
namespace remote_client
{

class DownloaderImpl : public DownloaderBase
{
    Q_OBJECT

public:
    DownloaderImpl(QString const& download_id,
                   QDBusUnixFileDescriptor fd,
                   std::shared_ptr<File> const& file,
                   ProviderInterface& provider);
    virtual ~DownloaderImpl();

    virtual std::shared_ptr<File> file() const override;
    virtual std::shared_ptr<QLocalSocket> socket() const override;
    virtual QFuture<void> finish_download() override;
    virtual QFuture<void> cancel() noexcept override;

    static std::shared_ptr<Downloader> make_downloader(QString const& upload_id,
                                                       QDBusUnixFileDescriptor fd,
                                                       std::shared_ptr<File> const& file,
                                                       ProviderInterface& provider);

private:
    QString download_id_;
    QDBusUnixFileDescriptor fd_;
    std::shared_ptr<File> file_;
    ProviderInterface& provider_;
    std::shared_ptr<QLocalSocket> read_socket_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
