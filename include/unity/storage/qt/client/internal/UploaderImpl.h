#pragma once

#include <unity/storage/common.h>
#include <unity/storage/qt/client/StorageSocket.h>

#include <QFile>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

#include <memory>

class QSocketNotifier;

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

// TODO: UploaderImpl and DownloaderImpl share a lot of code. Factor that out.

class UploaderImpl : public QObject
{
    Q_OBJECT

public:
    UploaderImpl(std::weak_ptr<File> file, ConflictPolicy policy);
    ~UploaderImpl();
    UploaderImpl(UploaderImpl const&) = delete;
    UploaderImpl& operator=(UploaderImpl const&) = delete;

    std::shared_ptr<File> file() const;
    std::shared_ptr<StorageSocket> socket() const;
    QFuture<TransferState> finish_upload();
    QFuture<void> cancel() noexcept;

private Q_SLOTS:
    void on_ready();
    void on_error();

private:
    void handle_error();
    void check_modified_time() const;

    enum State { connected, completed, finalized, cancelled, error };

    State state_ = connected;
    std::shared_ptr<File> file_;
    ConflictPolicy policy_;
    std::shared_ptr<StorageSocket> read_socket_;
    std::shared_ptr<StorageSocket> write_socket_;
    std::unique_ptr<QSocketNotifier> read_notifier_;
    std::unique_ptr<QSocketNotifier> error_notifier_;
    int fd_;
    QFile output_file_;
    char buf_[StorageSocket::CHUNK_SIZE];
    int end_ = 0;
    int pos_ = 0;
    bool eof_ = false;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
