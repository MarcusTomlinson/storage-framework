#include <unity/storage/qt/client/internal/DirectoryImpl.h>

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

QFuture<QVector<Item::UPtr>> DirectoryImpl::list() const
{
    return QFuture<QVector<Item::UPtr>>();
}

QFuture<QVector<Directory::UPtr>> DirectoryImpl::parents() const
{
    return QFuture<QVector<Directory::UPtr>>();
}

QFuture<Directory::UPtr> DirectoryImpl::create_dir(QString const& name)
{
    return QFuture<Directory::UPtr>();
}

QFuture<File::UPtr> DirectoryImpl::create_file(QString const& name)
{
    return QFuture<File::UPtr>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
