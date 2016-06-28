#pragma once

#include <unity/storage/qt/client/internal/FolderBase.h>
#include <unity/storage/qt/client/internal/local_client/ItemImpl.h>

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
namespace local_client
{

class FolderImpl : public virtual FolderBase, public virtual ItemImpl
{
public:
    FolderImpl(QString const& identity);
    FolderImpl(QString const& identity, ItemType type);

    QFuture<QVector<std::shared_ptr<Item>>> list() const override;
    QFuture<QVector<std::shared_ptr<Item>>> lookup(QString const& name) const override;
    QFuture<std::shared_ptr<Folder>> create_folder(QString const& name) override;
    QFuture<std::shared_ptr<Uploader>> create_file(QString const& name) override;

    static std::shared_ptr<Folder> make_folder(QString const& identity, std::weak_ptr<Root> root);
};

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
