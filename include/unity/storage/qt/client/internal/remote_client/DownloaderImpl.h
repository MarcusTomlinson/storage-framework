#pragma once

#include <unity/storage/qt/client/internal/DownloaderBase.h>

class QLocalSocket;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace remote_client
{

class DownloaderImpl : public DownloaderBase
{
    Q_OBJECT

public:
    DownloaderImpl(std::weak_ptr<File> file);
    ~DownloaderImpl();

    virtual std::shared_ptr<File> file() const override;
    virtual std::shared_ptr<QLocalSocket> socket() const override;
    virtual QFuture<TransferState> finish_download() override;
    virtual QFuture<void> cancel() noexcept override;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
