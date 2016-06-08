#include <unity/storage/qt/client/Folder.h>

#include <unity/storage/qt/client/internal/FolderBase.h>

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

Folder::Folder(FolderBase* p)
    : Item(p)
{
}

Folder::~Folder() = default;

QFuture<QVector<Item::SPtr>> Folder::list() const
{
    return dynamic_cast<FolderBase*>(p_.get())->list();
}

QFuture<Item::SPtr> Folder::lookup(QString const& name) const
{
    return dynamic_cast<FolderBase*>(p_.get())->lookup(name);
}

QFuture<Folder::SPtr> Folder::create_folder(QString const& name)
{
    return dynamic_cast<FolderBase*>(p_.get())->create_folder(name);
}

QFuture<shared_ptr<Uploader>> Folder::create_file(QString const& name)
{
    return dynamic_cast<FolderBase*>(p_.get())->create_file(name);
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
