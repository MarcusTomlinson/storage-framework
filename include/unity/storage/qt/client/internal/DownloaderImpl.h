#pragma once

#include <unity/storage/common.h>

#include <QFile>
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

class DownloaderImpl : public QObject
{
    Q_OBJECT

public:
    DownloaderImpl(std::weak_ptr<File> file);
    ~DownloaderImpl();
    DownloaderImpl(DownloaderImpl const&) = delete;
    DownloaderImpl& operator=(DownloaderImpl const&) = delete;

    std::shared_ptr<File> file() const;
    std::shared_ptr<QLocalSocket> socket() const;
    QFuture<TransferState> finish_download();
    QFuture<void> cancel() noexcept;

private Q_SLOTS:
    void on_ready();
    void on_bytes_written(qint64 bytes);
    void on_disconnected();
    void on_error();

private:
    void handle_error();

    enum State { in_progress, finalized, cancelled, error };

    State state_ = in_progress;
    std::shared_ptr<File> file_;
    std::shared_ptr<QLocalSocket> read_socket_;
    std::shared_ptr<QLocalSocket> write_socket_;
    std::unique_ptr<QFile> input_file_;
    QFutureInterface<TransferState> qf_;
    qint64 bytes_to_write_ = 0;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
