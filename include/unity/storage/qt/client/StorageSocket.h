#pragma once

#include <unity/storage/visibility.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QLocalSocket>
#pragma GCC diagnostic pop

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

class DownloaderImpl;
class UploaderImpl;

}  // namespace internal

/**
\brief Socket for downloading and uploading of file contents.
*/

class UNITY_STORAGE_EXPORT StorageSocket : public QLocalSocket
{
public:
    /// @cond
    virtual ~StorageSocket();
    /// @endcond

    /**
    \brief The service end of a StorageSocket reads and writes data in CHUNK_SIZE blocks.
    */
    static constexpr qint64 CHUNK_SIZE = 64 * 1024;

protected:
    virtual qint64 readData(char* data, qint64 c) override;
    virtual qint64 writeData(char const* data, qint64 c) override;

private:
    StorageSocket(QObject* parent = nullptr);

    friend class internal::DownloaderImpl;
    friend class internal::UploaderImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
