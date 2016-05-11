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

QFuture<void> RootImpl::destroy()
{
    // Cannot destroy root.
    throw StorageException();  // TODO
}

Account* RootImpl::account() const
{
    if (auto acc = account_.lock())
    {
        return acc.get();
    }
    throw DestroyedException();  // TODO
}

QFuture<int64_t> RootImpl::free_space_bytes() const
{
    using namespace boost::filesystem;

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
    return QFuture<Item::SPtr>();  // TODO
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
