#include <unity/storage/qt/client/internal/ItemImpl.h>

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

ItemImpl::ItemImpl() = default;

ItemImpl::~ItemImpl() = default;

QString ItemImpl::native_identity() const
{
    return "";
}

QString ItemImpl::name() const
{
    return "";
}

Root* ItemImpl::root() const
{
    return nullptr;
}

QVector<QString> ItemImpl::all_names() const
{
    return QVector<QString>();
}

QFuture<QVariantMap> ItemImpl::get_metadata() const
{
    return QFuture<QVariantMap>();
}

QFuture<QDateTime> ItemImpl::last_modified_time() const
{
    return QFuture<QDateTime>();
}

QFuture<QString> ItemImpl::mime_type() const
{
    return QFuture<QString>();
}

QFuture<void> ItemImpl::destroy()
{
    return QFuture<QString>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
