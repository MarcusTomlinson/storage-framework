#pragma once

#include <unity/storage/common.h>

#include <QFile>
#include <QFutureInterface>
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
class Uploader;

namespace internal
{

class UploaderImpl : public QObject
{
    Q_OBJECT

public:
    UploaderImpl(std::weak_ptr<File> file, ConflictPolicy policy);
    ~UploaderImpl();
    UploaderImpl(UploaderImpl const&) = delete;
    UploaderImpl& operator=(UploaderImpl const&) = delete;

    std::shared_ptr<File> file() const;
    std::shared_ptr<QLocalSocket> socket() const;
    QFuture<TransferState> finish_upload();
    QFuture<void> cancel() noexcept;

private Q_SLOTS:
    void on_ready();
    void on_disconnected();
    void on_error();

private:
    void finalize();
    void handle_error();
    void check_modified_time() const;

    enum State { in_progress, disconnected, finalized, cancelled, error };

    State state_ = in_progress;
    std::shared_ptr<File> file_;
    ConflictPolicy policy_;
    std::shared_ptr<QLocalSocket> read_socket_;
    std::shared_ptr<QLocalSocket> write_socket_;
    QFile output_file_;
    unity::util::ResourcePtr<int, std::function<void(int)>> tmp_fd_;
    bool eof_ = false;
    bool received_something_ = false;
    QFutureInterface<TransferState> qf_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
