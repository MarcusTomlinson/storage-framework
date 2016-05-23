#include <unity/storage/qt/client/internal/RootImpl.h>

#include <unity/storage/qt/client/Exceptions.h>

#include <boost/filesystem.hpp>

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

RootImpl::RootImpl(QString const& identity)
    : FolderImpl(identity, common::ItemType::root)
{
}

QString RootImpl::name() const
{
    return "/";
}

QFuture<QVector<Folder::SPtr>> RootImpl::parents() const
{
    QFutureInterface<QVector<Folder::SPtr>> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        return qf.future();
    }

    QVector<Folder::SPtr> results;
    results.append(root_.lock());
    qf.reportResult(results);
    return qf.future();
}

QFuture<void> RootImpl::destroy()
{
    // Cannot destroy root.
    throw StorageException();  // TODO
}

Account* RootImpl::account() const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }

    if (auto acc = account_.lock())
    {
        return acc.get();
    }
    throw RuntimeDestroyedException();
}

QFuture<int64_t> RootImpl::free_space_bytes() const
{
    using namespace boost::filesystem;

    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }

    QFutureInterface<int64_t> qf;
    try
    {
        space_info si = space(identity_.toStdString());
        qf.reportResult(si.available);
    }
    catch (std::exception const&)
    {
        qf.reportException(StorageException());  // TODO
    }
    return qf.future();
}

QFuture<int64_t> RootImpl::used_space_bytes() const
{
    using namespace boost::filesystem;

    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }

    QFutureInterface<int64_t> qf;
    try
    {
        space_info si = space(identity_.toStdString());
        qf.reportResult(si.capacity - si.available);
    }
    catch (std::exception const&)
    {
        qf.reportException(StorageException());  // TODO
    }
    return qf.future();
}

QFuture<Item::SPtr> RootImpl::get(QString native_identity) const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }

    return QFuture<Item::SPtr>();  // TODO
}

Root::SPtr RootImpl::make_root(QString const& identity)
{
    auto impl = new RootImpl(identity);
    Root::SPtr root(new Root(impl));
    impl->set_root(root);
    impl->set_public_instance(root);
    return root;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
