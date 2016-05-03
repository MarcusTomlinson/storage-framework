#pragma once

#include <unity/storage/qt/client/Directory.h>
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

class DirectoryImpl : public ItemImpl
{
public:
    DirectoryImpl() = default;
    ~DirectoryImpl() = default;
    DirectoryImpl(DirectoryImpl const&) = delete;
    DirectoryImpl& operator=(DirectoryImpl const&) = delete;

    QFuture<QVector<Item::UPtr>> list() const;
    QFuture<QVector<Directory::UPtr>> parents() const;
    QFuture<Directory::UPtr> create_dir(QString const& name);
    QFuture<File::UPtr> create_file(QString const& name);

private:
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
