#pragma once

#include <unity/storage/qt/client/internal/FileBase.h>
#include <unity/storage/qt/client/internal/remote_client/ItemImpl.h>

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

class FileImpl : public virtual FileBase, public virtual ItemImpl
{
public:
    FileImpl(QString const& identity);

    virtual QString name() const override;
    virtual int64_t size() const override;
    virtual QFuture<std::shared_ptr<Uploader>> create_uploader(ConflictPolicy policy) override;
    virtual QFuture<std::shared_ptr<Downloader>> create_downloader() override;

    static std::shared_ptr<File> make_file(QString const& identity, std::weak_ptr<Root> root);
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
