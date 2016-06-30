#include <unity/storage/qt/client/internal/UploaderBase.h>

#include <unity/storage/qt/client/Exceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

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

UploaderBase::UploaderBase(ConflictPolicy policy, int64_t size)
    : policy_(policy)
    , size_(size)
{
    if (size < 0)
    {
        throw InvalidArgumentException();  // TODO
    }
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
