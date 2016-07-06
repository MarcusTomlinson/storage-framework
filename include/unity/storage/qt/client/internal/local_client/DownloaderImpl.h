#pragma once

#include <unity/storage/qt/client/internal/DownloaderBase.h>

#include <QFile>
#include <QThread>

class QLocalSocket;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{
namespace local_client
{

class DownloadWorker : public QObject
{
    Q_OBJECT

public:
    DownloadWorker(int write_fd,
                   QString const& filename,
                   QFutureInterface<void>& qf,
                   QFutureInterface<void>& worker_initialized);
    void start_downloading() noexcept;

public Q_SLOTS:
    void do_finish();
    void do_cancel();

private Q_SLOTS:
    void on_bytes_written(qint64 bytes);
    void on_disconnected();
    void on_error();

private:
    void read_and_write_chunk();
    void handle_error(QString const& msg);

    enum State { in_progress, finalized, cancelled, error };

    State state_ = in_progress;
    int write_fd_;
    std::shared_ptr<QLocalSocket> write_socket_;
    QString filename_;
    std::unique_ptr<QFile> input_file_;
    QFutureInterface<void>& qf_;
    QFutureInterface<void>& worker_initialized_;
    qint64 bytes_to_write_;
    QString error_msg_;
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

class DownloaderImpl : public DownloaderBase
{
    Q_OBJECT

public:
    DownloaderImpl(std::weak_ptr<File> file);
    virtual ~DownloaderImpl();

    std::shared_ptr<File> file() const override;
    std::shared_ptr<QLocalSocket> socket() const override;
    QFuture<void> finish_download() override;
    QFuture<void> cancel() noexcept override;

Q_SIGNALS:
    void do_finish();
    void do_cancel();

private:
    std::shared_ptr<QLocalSocket> read_socket_;
    QFutureInterface<void> qf_;
    std::unique_ptr<DownloadThread> download_thread_;
    std::unique_ptr<DownloadWorker> worker_;
};

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
