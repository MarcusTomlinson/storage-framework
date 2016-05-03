#pragma once

#include <unity/storage/qt/client/Downloader.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/ItemImpl.h>
#include <unity/storage/qt/client/Uploader.h>

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

    int64_t size() const;
    QFuture<Uploader::UPtr> create_uploader(ConflictPolicy policy);
    QFuture<Downloader::UPtr> create_downloader();
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
