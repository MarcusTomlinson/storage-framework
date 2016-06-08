#include <unity/storage/qt/client/internal/FolderBase.h>

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

FolderBase::FolderBase(QString const& identity)
    : ItemBase(identity, ItemType::folder)
{
}

FolderBase::FolderBase(QString const& identity, ItemType type)
    : ItemBase(identity, type)
{
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
