#include <unity/storage/qt/client/internal/AccountImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/RootImpl.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <glib.h>
#pragma GCC diagnostic pop

#include <cassert>

#include <iostream>  // TODO: remove this

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
// ${STORAGE_FRAMEWORK_ROOT} or ${XDG_DATA_HOME} must exist and be a directory.
// If the storage-framework underneath that data directory does not exist, it is created.

string get_data_dir()
{
    char const* dir = getenv("STORAGE_FRAMEWORK_ROOT");
    if (!dir || *dir == '\0')
    {
        dir = g_get_user_data_dir();
    }

    boost::system::error_code ec;

    // The directory must exist.
    bool is_dir = boost::filesystem::is_directory(dir, ec);
    if (ec || !is_dir)
    {
        throw StorageException();  // TODO
    }

    // Create the storage-framework directory if it doesn't exist yet.
    string data_dir(dir);
    data_dir += "/storage-framework";
    if (!boost::filesystem::exists(data_dir))
    {
        boost::filesystem::create_directories(data_dir, ec);
        if (ec)
        {
            throw StorageException();  // TODO
        }
    }
    return data_dir;
}

}  // namespace

AccountImpl::AccountImpl(QString const& owner,
                         QString const& owner_id,
                         QString const& description)
    : owner_(owner)
    , owner_id_(owner_id)
    , description_(description)
    , copy_in_progress_(false)
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

QFuture<QVector<Root::SPtr>> AccountImpl::roots()
{
    using namespace boost::filesystem;

    QFutureInterface<QVector<Root::SPtr>> qf;

    if (!roots_.isEmpty())
    {
        qf.reportResult(roots_);
        qf.reportFinished();
        return qf.future();
    }

    try
    {
        // Create the root on first access.
        auto rpath = canonical(get_data_dir()).native();
        auto root = RootImpl::make_root(QString::fromStdString(rpath), public_instance_);
        roots_.append(root);
        qf.reportResult(roots_);
    }
    catch (std::exception const&)
    {
        qf.reportException(StorageException());  // TODO
    }
    qf.reportFinished();
    return qf.future();
}

void AccountImpl::set_runtime(weak_ptr<Runtime> p)
{
    runtime_ = p;
}

void AccountImpl::set_public_instance(weak_ptr<Account> p)
{
    assert(p.lock());
    public_instance_ = p;
}

AccountImpl::RecursiveCopyGuard AccountImpl::get_copy_guard()
{
    return RecursiveCopyGuard(this);
}

AccountImpl::RecursiveCopyGuard::RecursiveCopyGuard(AccountImpl* account)
    : account_(account)
{
    if (account_->copy_in_progress_.exchange(true))
    {
        throw StorageException();  // TODO
    }
}

AccountImpl::RecursiveCopyGuard::~RecursiveCopyGuard()
{
    account_->copy_in_progress_ = false;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
