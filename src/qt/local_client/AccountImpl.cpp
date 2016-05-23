#include <unity/storage/qt/client/internal/AccountImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/Root.h>
#include <unity/storage/qt/client/internal/RootImpl.h>

#include <boost/filesystem.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <glib.h>
#pragma GCC diagnostic pop

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

namespace
{

// Return ${STORAGE_FRAMEWORK_ROOT}/storage-framework. If STORAGE_FRAMEWORK_ROOT
// is not set, return ${XDG_DATA_HOME}/storage-framework.

string get_data_dir()
{
    char const* dir = getenv("STORAGE_FRAMEWORK_ROOT");
    if (!dir || *dir == '\0')
    {
        dir = g_get_user_data_dir();
    }
    string data_dir(dir);
    data_dir += "/storage-framework";
    return data_dir;
}

}  // namespace

AccountImpl::AccountImpl(QString const& owner,
                         QString const& owner_id,
                         QString const& description)
    : owner_(owner)
    , owner_id_(owner_id)
    , description_(description)
{
    assert(!owner.isEmpty());
    assert(!owner_id.isEmpty());
    assert(!description.isEmpty());
}

Runtime* AccountImpl::runtime() const
{
    if (auto runtime = runtime_.lock())
    {
        return runtime.get();
    }
    throw StorageException();  // TODO
}

QString AccountImpl::owner() const
{
    return owner_;
}

QString AccountImpl::owner_id() const
{
    return owner_id_;
}

QString AccountImpl::description() const
{
    return description_;
}

QFuture<QVector<Root::SPtr>> AccountImpl::get_roots()
{
    using namespace boost::filesystem;

    QFutureInterface<QVector<Root::SPtr>> qf;

    if (roots_.isEmpty())
    {
        // Create the root on first access.
        auto rpath = canonical(get_data_dir()).native();
        auto impl = new RootImpl(QString::fromStdString(rpath));
        Root::SPtr root(new Root(impl));
        impl->set_root(root);
        impl->set_public_instance(root);
        try
        {
            file_status st = status(roots_[0]->native_identity().toStdString());
            if (!is_directory(st))
            {
                qf.reportException(StorageException());  // TODO
                return qf.future();
            }
        }
        catch (std::exception const&)
        {
            qf.reportException(StorageException());  // TODO
            return qf.future();
        }
        roots_.append(root);
    }

    qf.reportResult(roots_);
    return qf.future();
}

void AccountImpl::set_runtime(weak_ptr<Runtime> p)
{
    runtime_ = p;
}

void AccountImpl::set_public_instance(weak_ptr<Account> p)
{
    public_instance_ = p;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
