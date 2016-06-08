#include <unity/storage/qt/client/internal/remote_client/ItemImpl.h>

#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>

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
namespace remote_client
{

ItemImpl::ItemImpl(QString const& identity, ItemType type)
    : ItemBase(identity, type)
{
}

ItemImpl::~ItemImpl() = default;

QString ItemImpl::name() const
{
    return "";
}

QVariantMap ItemImpl::metadata() const
{
    return QVariantMap();
}

QDateTime ItemImpl::last_modified_time() const
{
    return QDateTime();
}

QFuture<shared_ptr<Item>> ItemImpl::copy(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    return QFuture<shared_ptr<Item>>();
}

QFuture<shared_ptr<Item>> ItemImpl::move(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    return QFuture<shared_ptr<Item>>();
}

QFuture<QVector<Folder::SPtr>> ItemImpl::parents() const
{
    return QFuture<QVector<Folder::SPtr>>();
}

QVector<QString> ItemImpl::parent_ids() const
{
    QVector<QString>();
}

QFuture<void> ItemImpl::destroy()
{
    return QFuture<void>();
}

bool ItemImpl::equal_to(ItemBase const& other) const noexcept
{
    return false;
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
