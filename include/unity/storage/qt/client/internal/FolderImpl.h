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
    FolderImpl() = default;
    ~FolderImpl() = default;
    FolderImpl(FolderImpl const&) = delete;
    FolderImpl& operator=(FolderImpl const&) = delete;

    virtual QFuture<void> destroy() override;

    QFuture<QVector<Item::SPtr>> list() const;
    QFuture<QVector<Item::SPtr>> lookup(QString const& name) const;
    QFuture<QVector<Folder::SPtr>> parents() const;
    QFuture<Folder::SPtr> create_folder(QString const& name);
    QFuture<std::shared_ptr<Uploader>> create_file(QString const& name);

private:
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
