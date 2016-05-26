#pragma once

#include <unity/storage/common.h>
#include <unity/storage/qt/client/StorageSocket.h>

#include <QFile>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

#include <atomic>
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

class DownloaderImpl : public QObject, public std::enable_shared_from_this<DownloaderImpl>
{
    Q_OBJECT

public:
    DownloaderImpl(std::weak_ptr<File> file);
    ~DownloaderImpl();
    DownloaderImpl(DownloaderImpl const&) = delete;
    DownloaderImpl& operator=(DownloaderImpl const&) = delete;

    std::shared_ptr<File> file() const;
    std::shared_ptr<StorageSocket> socket() const;
    QFuture<TransferState> finish_download();
    QFuture<void> cancel() noexcept;

private Q_SLOTS:
    void on_ready();
    void on_error();

private:
    void handle_error();

    enum State { connected, finalized, cancelled, error };

    std::atomic<State> state_;
    std::shared_ptr<File> file_;                       // Immutable
    std::shared_ptr<StorageSocket> read_socket_;       // Immutable
    std::shared_ptr<StorageSocket> write_socket_;      // Immutable
    std::unique_ptr<QSocketNotifier> write_notifier_;  // Immutable
    std::unique_ptr<QSocketNotifier> error_notifier_;  // Immutable
    QFile input_file_;                                 // Immutable
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
