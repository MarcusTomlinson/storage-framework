#include <unity/storage/qt/client/StorageSocket.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

StorageSocket::StorageSocket(QObject* parent)
    : QLocalSocket(parent)
{
}

StorageSocket::~StorageSocket() = default;

qint64 StorageSocket::readData(char* data, qint64 c)
{
    return QLocalSocket::readData(data, c);
}

qint64 StorageSocket::writeData(char const* data, qint64 c)
{
    return QLocalSocket::writeData(data, c);
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
