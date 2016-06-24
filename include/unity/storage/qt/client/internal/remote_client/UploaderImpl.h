#pragma once

#include <unity/storage/qt/client/internal/UploaderBase.h>

#include <QLocalSocket>
#include <QString>

class ProviderInterface;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Root;
class Uploader;

namespace internal
{
namespace remote_client
{

class UploaderImpl : public UploaderBase
{
    Q_OBJECT

public:
    UploaderImpl(QString upload_id,
                 int fd,
                 QString const& old_etag,
                 std::weak_ptr<Root> root,
                 ProviderInterface& provider);
    ~UploaderImpl();

    virtual std::shared_ptr<QLocalSocket> socket() const override;
    virtual QFuture<std::shared_ptr<File>> finish_upload() override;
    virtual QFuture<void> cancel() noexcept override;

    static std::shared_ptr<Uploader> make_uploader(QString const& upload_id,
                                                   int fd,
                                                   QString const& old_etag,
                                                   std::weak_ptr<Root> root,
                                                   ProviderInterface& provider);

private:
    QString upload_id_;
    QString old_etag_;
    std::weak_ptr<Root> root_;
    ProviderInterface& provider_;
    std::shared_ptr<QLocalSocket> write_socket_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
