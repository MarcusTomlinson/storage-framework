#pragma once

#include <unity/storage/qt/client/Root.h>
#include <unity/storage/qt/client/internal/DirectoryImpl.h>

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

class RootImpl : public DirectoryImpl
{
public:
    RootImpl() = default;
    ~RootImpl() = default;
    RootImpl(RootImpl const&) = delete;
    RootImpl& operator=(RootImpl const&) = delete;

    Account* account() const;
    QFuture<int64_t> free_space_bytes() const;
    QFuture<int64_t> used_space_bytes() const;
    QFuture<Item::UPtr> get(QString native_identity) const;

private:
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
