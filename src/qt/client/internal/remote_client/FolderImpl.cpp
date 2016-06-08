#include <unity/storage/qt/client/internal/remote_client/FolderImpl.h>

//#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/Item.h>
//#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/Folder.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>

#include <QtConcurrent>

#include <fcntl.h>

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
namespace remote_client
{

FolderImpl::FolderImpl(QString const& identity)
    : ItemBase(identity, ItemType::folder)
    , FolderBase(identity, ItemType::folder)
    , ItemImpl(identity, ItemType::folder)
{
}

FolderImpl::FolderImpl(QString const& identity, ItemType type)
    : ItemBase(identity, type)
    , FolderBase(identity, type)
    , ItemImpl(identity, type)
{
}

QFuture<QVector<Item::SPtr>> FolderImpl::list() const
{
    return QFuture<QVector<Item::SPtr>>();
}

QFuture<Item::SPtr> FolderImpl::lookup(QString const& name) const
{
    return QFuture<Item::SPtr>();
}

QFuture<Folder::SPtr> FolderImpl::create_folder(QString const& name)
{
    return QFuture<Folder::SPtr>();
}

QFuture<shared_ptr<Uploader>> FolderImpl::create_file(QString const& name)
{
    return QFuture<shared_ptr<Uploader>> ();
}

Folder::SPtr FolderImpl::make_folder(QString const& identity, weak_ptr<Root> root)
{
    auto impl = new FolderImpl(identity);
    Folder::SPtr folder(new Folder(impl));
    impl->set_root(root);
    impl->set_public_instance(folder);
    return folder;
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
