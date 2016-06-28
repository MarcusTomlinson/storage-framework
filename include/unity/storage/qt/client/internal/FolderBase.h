#pragma once

#include <unity/storage/qt/client/internal/ItemBase.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Uploader;

namespace internal
{

class FolderBase : public virtual ItemBase
{
public:
    FolderBase(QString const& identity);
    FolderBase(QString const& identity, ItemType type);

    virtual QFuture<QVector<std::shared_ptr<Item>>> list() const = 0;
    virtual QFuture<QVector<std::shared_ptr<Item>>> lookup(QString const& name) const = 0;
    virtual QFuture<std::shared_ptr<Folder>> create_folder(QString const& name) = 0;
    virtual QFuture<std::shared_ptr<Uploader>> create_file(QString const& name) = 0;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
