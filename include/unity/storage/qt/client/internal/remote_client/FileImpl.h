#pragma once

#include <unity/storage/qt/client/internal/FileBase.h>
#include <unity/storage/qt/client/internal/remote_client/ItemImpl.h>

namespace unity
{
namespace storage
{
namespace internal
{

class ItemMetadata;

}  // namespace internal

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
    FileImpl(storage::internal::ItemMetadata const& md);

    virtual int64_t size() const override;
    virtual QFuture<std::shared_ptr<Uploader>> create_uploader(ConflictPolicy policy, int64_t size) override;
    virtual QFuture<std::shared_ptr<Downloader>> create_downloader() override;

    static std::shared_ptr<File> make_file(storage::internal::ItemMetadata const& md, std::weak_ptr<Root> root);
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
