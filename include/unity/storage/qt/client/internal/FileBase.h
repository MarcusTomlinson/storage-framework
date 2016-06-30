#pragma once

#include <unity/storage/qt/client/internal/ItemBase.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Downloader;
class File;
class Uploader;

namespace internal
{

class FileBase : public virtual ItemBase
{
public:
    FileBase(QString const& identity);

    virtual int64_t size() const = 0;
    virtual QFuture<std::shared_ptr<Uploader>> create_uploader(ConflictPolicy policy, int64_t size) = 0;
    virtual QFuture<std::shared_ptr<Downloader>> create_downloader() = 0;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
