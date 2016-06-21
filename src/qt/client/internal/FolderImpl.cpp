#include <unity/storage/qt/client/internal/FolderImpl.h>

using namespace std;

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

QFuture<QVector<Item::SPtr>> FolderImpl::list() const
{
    return QFuture<QVector<Item::SPtr>>();
}

QFuture<Item::SPtr> FolderImpl::lookup(QString const&) const
{
    return QFuture<Item::SPtr>();
}

QFuture<Folder::SPtr> FolderImpl::create_folder(QString const& name)
{
    return QFuture<Folder::SPtr>();
}

QFuture<shared_ptr<Uploader>> FolderImpl::create_file(QString const& name)
{
    return QFuture<shared_ptr<Uploader>>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
