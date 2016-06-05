#pragma once

#include <unity/storage/common.h>

#include <QFile>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop
#include <QFutureInterface>
#include <QThread>

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

class DownloadWorker : public QObject
{
    Q_OBJECT

public:
    DownloadWorker(int write_fd, QString const& filename, QFutureInterface<TransferState>& qf);
    void start_downloading();

public Q_SLOTS:
    void do_finish();
    void do_cancel();

private Q_SLOTS:
    void write_chunk();
    void on_bytes_written(qint64 bytes);
    void on_disconnected();
    void on_error();

private:
    void handle_error();

    enum State { in_progress, finalized, cancelled, error };

    State state_ = in_progress;
    int write_fd_;
    std::shared_ptr<QLocalSocket> write_socket_;
    QString filename_;
    std::unique_ptr<QFile> input_file_;
    QFutureInterface<TransferState>& qf_;
    qint64 bytes_to_write_;
};

class DownloadThread : public QThread
{
    Q_OBJECT

public:
    DownloadThread(DownloadWorker* worker);
    virtual void run() override;

private:
    DownloadWorker* worker_;
};

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

Q_SIGNALS:
    void do_finish();
    void do_cancel();

private:
    std::shared_ptr<File> file_;
    std::shared_ptr<QLocalSocket> read_socket_;
    QFutureInterface<TransferState> qf_;
    std::unique_ptr<DownloadThread> download_thread_;
    std::unique_ptr<DownloadWorker> worker_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
