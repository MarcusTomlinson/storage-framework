#include <unity/storage/qt/client/internal/FileBase.h>

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

FileBase::FileBase(QString const& identity)
    : ItemBase(identity, ItemType::file)
{
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
