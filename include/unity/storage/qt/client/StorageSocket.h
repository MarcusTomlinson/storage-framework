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

class UNITY_STORAGE_EXPORT StorageSocket : public QLocalSocket
{
public:
    StorageSocket(QObject* parent = nullptr);
    virtual ~StorageSocket();

protected:
    virtual qint64 readData(char* data, qint64 c) override;
    virtual qint64 writeData(char const* data, qint64 c) override;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
