#pragma once

#include <unity/storage/common.h>

#include <QFile>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop
#include <QFutureInterface>
#include <QThread>
#include <unity/util/ResourcePtr.h>

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
    Q_OBJECT

public:
    UploaderBase(std::weak_ptr<File> file, ConflictPolicy policy);
    UploaderBase(UploaderBase&) = delete;
    UploaderBase& operator=(UploaderBase const&) = delete;

    virtual std::shared_ptr<File> file() const = 0;
    virtual std::shared_ptr<QLocalSocket> socket() const = 0;
    virtual QFuture<TransferState> finish_upload() = 0;
    virtual QFuture<void> cancel() noexcept = 0;

protected:
    std::shared_ptr<File> file_;
    ConflictPolicy policy_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
