#pragma once

#include <unity/storage/common.h>
#include <unity/storage/qt/client/StorageSocket.h>

#include <QFile>
#include <QFutureInterface>

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
class Uploader;

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
    void on_write(qint64 bytes_written);
    void on_error();
    void on_disconnect();

private:
    void finalize();
    void handle_error();
    void check_modified_time() const;

    enum State { connected, disconnected, finalized, cancelled, error };

    State state_;
    std::shared_ptr<File> file_;
    ConflictPolicy policy_;
    std::shared_ptr<StorageSocket> read_socket_;
    std::shared_ptr<StorageSocket> write_socket_;
    std::unique_ptr<QSocketNotifier> read_notifier_;
    std::unique_ptr<QSocketNotifier> error_notifier_;
    int fd_;
    QFile output_file_;
    bool disconnected_ = false;
    bool eof_ = false;
    bool bytes_written_ = false;
    QFutureInterface<TransferState> qf_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
