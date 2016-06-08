#include <unity/storage/qt/client/internal/UploaderBase.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

#include <cassert>

using namespace std;

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

UploaderBase::UploaderBase(weak_ptr<File> file, ConflictPolicy policy)
    : file_(file)
    , policy_(policy)
{
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
