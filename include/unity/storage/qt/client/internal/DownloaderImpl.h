#pragma once

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

class DownloaderImpl
{
public:
    DownloaderImpl(std::weak_ptr<File> file);
    ~DownloaderImpl();
    DownloaderImpl(DownloaderImpl const&) = delete;
    DownloaderImpl& operator=(DownloaderImpl const&) = delete;

    std::shared_ptr<File> file() const;
    std::shared_ptr<QLocalSocket> socket() const;
    QFuture<void> close();
    QFuture<void> cancel();

private:
    enum State { uninitialized, connected, finalized };

    State state_ = uninitialized;
    std::shared_ptr<File> file_;
    std::shared_ptr<QLocalSocket> read_socket_;
    std::shared_ptr<QLocalSocket> write_socket_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
