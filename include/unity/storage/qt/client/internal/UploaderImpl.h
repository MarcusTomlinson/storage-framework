#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

#include <memory>

class QLocalSocket;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class File;

namespace internal
{

class UploaderImpl
{
public:
    UploaderImpl() = default;
    ~UploaderImpl() = default;
    UploaderImpl(UploaderImpl const&) = delete;
    UploaderImpl& operator=(UploaderImpl const&) = delete;

    std::shared_ptr<File> file() const;
    std::shared_ptr<QLocalSocket> socket() const;
    QFuture<void> finish_upload();
    QFuture<void> cancel();
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
