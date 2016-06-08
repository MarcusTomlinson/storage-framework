#pragma once

#include <unity/storage/qt/client/internal/UploaderBase.h>

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

class UploaderImpl : public UploaderBase
{
    Q_OBJECT

public:
    UploaderImpl(std::weak_ptr<File> file, ConflictPolicy policy);
    ~UploaderImpl();

    virtual std::shared_ptr<File> file() const override;
    virtual std::shared_ptr<QLocalSocket> socket() const override;
    virtual QFuture<TransferState> finish_upload() override;
    virtual QFuture<void> cancel() noexcept override;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
