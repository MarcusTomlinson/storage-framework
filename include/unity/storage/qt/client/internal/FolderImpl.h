#pragma once

#include <unity/storage/qt/client/Folder.h>
#include <unity/storage/qt/client/internal/ItemImpl.h>

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

class FolderImpl : public ItemImpl
{
public:
    FolderImpl(QString const& identity);
    FolderImpl(QString const& identity, ItemType type);
    ~FolderImpl();
    FolderImpl(FolderImpl const&) = delete;
    FolderImpl& operator=(FolderImpl const&) = delete;

    virtual QFuture<void> destroy() override;

    QFuture<QVector<Item::SPtr>> list() const;
    QFuture<Item::SPtr> lookup(QString const& name) const;
    QFuture<Folder::SPtr> create_folder(QString const& name);
    QFuture<std::shared_ptr<Uploader>> create_file(QString const& name);

    static std::shared_ptr<Folder> make_folder(QString const& identity, std::weak_ptr<Root> root);
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
