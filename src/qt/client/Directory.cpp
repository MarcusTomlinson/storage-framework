#include <unity/storage/qt/client/Directory.h>

#include <unity/storage/qt/client/internal/DirectoryImpl.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

using namespace internal;

Directory::Directory(DirectoryImpl* p)
    : Item(p)
{
}

Directory::~Directory() = default;

QFuture<QVector<Item::UPtr>> Directory::list() const
{
    return static_cast<DirectoryImpl*>(p_.get())->list();
}

QFuture<QVector<Directory::UPtr>> Directory::parents() const
{
    return static_cast<DirectoryImpl*>(p_.get())->parents();
}

QFuture<Directory::UPtr> Directory::create_dir(QString const& name)
{
    return static_cast<DirectoryImpl*>(p_.get())->create_dir(name);
}

QFuture<File::UPtr> Directory::create_file(QString const& name)
{
    return static_cast<DirectoryImpl*>(p_.get())->create_file(name);
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
