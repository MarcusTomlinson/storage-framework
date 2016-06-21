#pragma once

#include <unity/storage/common.h>

#include <QFile>
#include <QFutureInterface>
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

class UploadWorker : public QObject
{
    Q_OBJECT

public:
    UploadWorker(int read_fd,
                 std::shared_ptr<File> const& file,
                 ConflictPolicy policy,
                 QFutureInterface<TransferState>& qf,
                 QFutureInterface<void>& worker_initialized);
    void start_uploading() noexcept;

public Q_SLOTS:
    void do_finish();
    void do_cancel();

private Q_SLOTS:
    void on_bytes_ready();
    void on_read_channel_finished();
    void on_error();

private:
    void read_and_write_chunk();
    void finalize();
    void handle_error();
    void check_modified_time() const;

    enum State { in_progress, finalized, cancelled, error };

    State state_ = in_progress;
    int read_fd_;
    std::shared_ptr<QLocalSocket> read_socket_;
    std::shared_ptr<File> file_;
    std::unique_ptr<QFile> output_file_;
    unity::util::ResourcePtr<int, std::function<void(int)>> tmp_fd_;
    ConflictPolicy policy_;
    QFutureInterface<TransferState>& qf_;
    QFutureInterface<void>& worker_initialized_;
    bool disconnected_ = false;
};

class UploadThread : public QThread
{
    Q_OBJECT

public:
    UploadThread(UploadWorker* worker);
    virtual void run() override;

private:
    UploadWorker* worker_;
};

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

Q_SIGNALS:
    void do_finish();
    void do_cancel();

private Q_SLOTS:
    void disconnected();

private:
    std::shared_ptr<File> file_;
    ConflictPolicy policy_;
    std::shared_ptr<QLocalSocket> write_socket_;
    QFutureInterface<TransferState> qf_;
    std::unique_ptr<UploadThread> upload_thread_;
    std::unique_ptr<UploadWorker> worker_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
