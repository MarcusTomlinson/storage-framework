#include <unity/storage/qt/client/File.h>

#include <unity/storage/qt/client/internal/FileBase.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

using namespace internal;

File::File(FileBase* p)
    : Item(p)
{
}

File::~File() = default;

int64_t File::size() const
{
    return dynamic_cast<FileBase*>(p_.get())->size();
}

QFuture<shared_ptr<Uploader>> File::create_uploader(ConflictPolicy policy, int64_t size)
{
    return dynamic_cast<FileBase*>(p_.get())->create_uploader(policy, size);
}

QFuture<shared_ptr<Downloader>> File::create_downloader()
{
    return dynamic_cast<FileBase*>(p_.get())->create_downloader();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
