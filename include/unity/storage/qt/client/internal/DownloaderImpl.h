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

class DownloaderImpl : public QObject
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
    void on_disconnect();

private:
    void handle_error();

    enum State { connected, finalized, cancelled, error };

    State state_ = connected;
    std::shared_ptr<File> file_;
    std::shared_ptr<StorageSocket> read_socket_;
    std::shared_ptr<StorageSocket> write_socket_;
    std::unique_ptr<QSocketNotifier> write_notifier_;
    std::unique_ptr<QSocketNotifier> error_notifier_;
    QFile input_file_;
    char buf_[StorageSocket::CHUNK_SIZE];
    int end_ = 0;
    int pos_ = 0;
    bool eof_ = false;
    QFutureInterface<TransferState> qf_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
