#include <unity/storage/qt/client/internal/ItemImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/Root.h>

#include <boost/filesystem.hpp>

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
{
    assert(!identity.isEmpty());
    auto path = boost::filesystem::canonical(identity.toStdString());
    assert(path.is_absolute());
    identity_ = QString::fromStdString(path.native());
    name_ = QString::fromStdString(path.filename().native());
}

ItemImpl::~ItemImpl() = default;

QString ItemImpl::native_identity() const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }
    return identity_;
}

QString ItemImpl::name() const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }
    return name_;
}

Root* ItemImpl::root() const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }

    if (auto r = root_.lock())
    {
        return r.get();
    }
    throw DestroyedException();  // TODO
}

QFuture<QVariantMap> ItemImpl::get_metadata() const
{
    QFutureInterface<QVariantMap> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());  // TODO
        return qf.future();
    }
    return QFuture<QVariantMap>();  // TODO
}

QFuture<QDateTime> ItemImpl::last_modified_time() const
{
    QFutureInterface<QDateTime> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());  // TODO
        return qf.future();
    }

    try
    {
        auto mtime = boost::filesystem::last_write_time(identity_.toStdString());
        QDateTime dt;
        dt.setTime_t(mtime);
        qf.reportResult(dt);
    }
    catch (std::exception const&)
    {
        qf.reportException(StorageException());  // TODO
    }
    return qf.future();
}

common::ItemType ItemImpl::type() const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }

    return type_;
}

QFuture<shared_ptr<Item>> copy(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    return QFuture<shared_ptr<Item>>();  // TODO
}

QFuture<shared_ptr<Item>> move(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    return QFuture<shared_ptr<Item>>();  // TODO
}

QFuture<void> ItemImpl::destroy()
{
    QFutureInterface<void> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());  // TODO
        return qf.future();
    }
    return QFuture<QString>();  // TODO
}

void ItemImpl::set_root(weak_ptr<Root> p)
{
    assert(p.lock());
    root_ = p;
}

void ItemImpl::set_public_instance(weak_ptr<Item> p)
{
    assert(public_instance_.lock());
    public_instance_ = p;
}

weak_ptr<Item> ItemImpl::public_instance() const
{
    assert(public_instance_.lock());
    return public_instance_;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
