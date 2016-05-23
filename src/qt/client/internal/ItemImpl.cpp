#include <unity/storage/qt/client/internal/ItemImpl.h>

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

ItemImpl::ItemImpl(QString const& identity)
    : identity_(identity)
{
    assert(!identity.isEmpty());
}

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

common::ItemType ItemImpl::type() const
{
    return QFuture<common::ItemType>();
}

QFuture<QVariantMap> ItemImpl::get_metadata() const
{
    return QFuture<QVariantMap>();
}

QFuture<QDateTime> ItemImpl::last_modified_time() const
{
    return QFuture<QDateTime>();
}

QFuture<shared_ptr<Item>> copy(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    return QFuture<shared_ptr<Item>>();
}

QFuture<shared_ptr<Item>> move(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    return QFuture<shared_ptr<Item>>();
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
