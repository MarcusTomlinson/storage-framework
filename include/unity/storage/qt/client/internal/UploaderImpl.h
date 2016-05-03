#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

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

class UploaderImpl
{
public:
    UploaderImpl() = default;
    ~UploaderImpl() = default;
    UploaderImpl(UploaderImpl const&) = delete;
    UploaderImpl& operator=(UploaderImpl const&) = delete;

    QFuture<int> fd() const;
    QFuture<void> close();
    QFuture<void> cancel();
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
