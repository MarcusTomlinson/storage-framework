#pragma once

#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/ItemImpl.h>

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

class FileImpl : public ItemImpl
{
public:
    FileImpl() = default;
    ~FileImpl() = default;
    FileImpl(FileImpl const&) = delete;
    FileImpl& operator=(FileImpl const&) = delete;

    virtual QFuture<void> destroy() override;

    int64_t size() const;
    QFuture<std::shared_ptr<Uploader>> create_uploader(ConflictPolicy policy);
    QFuture<std::shared_ptr<Downloader>> create_downloader();
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
