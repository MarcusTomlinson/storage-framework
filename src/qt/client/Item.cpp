#include <unity/storage/qt/client/Item.h>

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

Item::Item(internal::ItemImpl* p)
    : p_(p)
{
    assert(p != nullptr);
}

Item::~Item() = default;

QString Item::native_identity() const
{
    return p_->native_identity();
}

QString Item::name() const
{
    return p_->name();
}

Root* Item::root() const
{
    return p_->root();
}

QVector<QString> Item::all_names() const
{
    return p_->all_names();
}

QFuture<QVariantMap> Item::get_metadata() const
{
    return p_->get_metadata();
}

QFuture<QDateTime> Item::last_modified_time() const
{
    return p_->last_modified_time();
}

QFuture<QString> Item::mime_type() const
{
    return p_->mime_type();
}

QFuture<void> Item::destroy()
{
    return p_->destroy();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
