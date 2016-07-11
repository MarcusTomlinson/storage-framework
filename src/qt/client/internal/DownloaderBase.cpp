#include <unity/storage/qt/client/internal/DownloaderBase.h>

#include <cassert>

using namespace std;

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

DownloaderBase::DownloaderBase(weak_ptr<File> file)
    : file_(file.lock())
{
    assert(file_);
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
