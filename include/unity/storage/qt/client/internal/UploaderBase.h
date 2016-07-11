#pragma once

#include <unity/storage/common.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wswitch-default"
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

class UploaderBase : public QObject
{
public:
    UploaderBase(ConflictPolicy policy);
    UploaderBase(UploaderBase&) = delete;
    UploaderBase& operator=(UploaderBase const&) = delete;

    virtual std::shared_ptr<QLocalSocket> socket() const = 0;
    virtual QFuture<std::shared_ptr<File>> finish_upload() = 0;
    virtual QFuture<void> cancel() noexcept = 0;

protected:
    ConflictPolicy policy_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
