#include <unity/storage/qt/client/Folder.h>

#include <unity/storage/qt/client/internal/FolderImpl.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

using namespace internal;
using namespace std;

Folder::Folder(FolderImpl* p)
    : Item(p)
{
}

Folder::~Folder() = default;

QFuture<QVector<Item::SPtr>> Folder::list() const
{
    return static_cast<FolderImpl*>(p_.get())->list();
}

QFuture<QVector<Item::SPtr>> Folder::lookup(QString const& name) const
{
    return static_cast<FolderImpl*>(p_.get())->lookup(name);
}

QFuture<QVector<Folder::SPtr>> Folder::parents() const
{
    return static_cast<FolderImpl*>(p_.get())->parents();
}

QFuture<Folder::SPtr> Folder::create_folder(QString const& name)
{
    return static_cast<FolderImpl*>(p_.get())->create_folder(name);
}

QFuture<shared_ptr<Uploader>> Folder::create_file(QString const& name)
{
    return static_cast<FolderImpl*>(p_.get())->create_file(name);
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
