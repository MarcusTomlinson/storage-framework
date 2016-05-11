#include <unity/storage/qt/client/File.h>

#include <unity/storage/qt/client/internal/FileImpl.h>

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

File::File(FileImpl* p)
    : Item(p)
{
}

File::~File() = default;

int64_t File::size() const
{
    return static_cast<FileImpl*>(p_.get())->size();
}

QFuture<shared_ptr<Uploader>> File::create_uploader(ConflictPolicy policy)
{
    return static_cast<FileImpl*>(p_.get())->create_uploader(policy);
}

QFuture<shared_ptr<Downloader>> File::create_downloader()
{
    return static_cast<FileImpl*>(p_.get())->create_downloader();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
