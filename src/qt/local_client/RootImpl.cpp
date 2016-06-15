#include <unity/storage/qt/client/internal/RootImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/FileImpl.h>

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

RootImpl::RootImpl(QString const& identity, weak_ptr<Account> const& account)
    : FolderImpl(identity, ItemType::root)
    , account_(account)
{
    using namespace boost::filesystem;

    path id_path = path(identity.toStdString());
    if (!id_path.is_absolute())
    {
        throw StorageException();  // TODO
    }
    path can_path = canonical(id_path);
    auto id_len = std::distance(id_path.begin(), id_path.end());
    auto can_len = std::distance(can_path.begin(), can_path.end());
    if (id_len != can_len)
    {
        // identity denotes a weird path that we won't trust because
        // it might contain ".." or similar.
        throw StorageException();  // TODO
    }
    assert(account.lock());
}

QString RootImpl::name() const
{
    return "";
}

QFuture<QVector<Folder::SPtr>> RootImpl::parents() const
{
    QFutureInterface<QVector<Folder::SPtr>> qf;
    qf.reportResult(QVector<Folder::SPtr>());  // For the root, we return an empty vector.
    qf.reportFinished();
    return qf.future();
}

QVector<QString> RootImpl::parent_ids() const
{
    return QVector<QString>();  // For the root, we return an empty vector.
}

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
    throw RuntimeDestroyedException();
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
    qf.reportFinished();
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
    qf.reportFinished();
    return qf.future();
}

QFuture<Item::SPtr> RootImpl::get(QString native_identity) const
{
    using namespace boost::filesystem;

    QFutureInterface<Item::SPtr> qf;
    try
    {
        path id_path = native_identity.toStdString();
        if (!id_path.is_absolute())
        {
            throw StorageException();  // TODO
        }

        // Make sure that native_identity is contained in or equal to the root path.
        id_path = canonical(id_path);
        auto root_path = path(root()->native_identity().toStdString());
        auto id_len = std::distance(id_path.begin(), id_path.end());
        auto root_len = std::distance(root_path.begin(), root_path.end());
        if (id_len < root_len)
        {
            // native_identity can't possibly point at something below the root.
            throw StorageException();  // TODO
        }
        if (!std::equal(root_path.begin(), root_path.end(), id_path.begin()))
        {
            // id_path differs from root path in some path component, so id_path
            // does not point at a location that's contained in root_path.
            throw StorageException();
        }

        file_status s = status(id_path);
        QString path = QString::fromStdString(id_path.native());
        if (is_directory(s))
        {
            if (id_path == root_path)
            {
                qf.reportResult(make_root(path, account_));
            }
            else
            {
                qf.reportResult(make_folder(path, root_));
            }
        }
        else if (is_regular_file(s))
        {
            qf.reportResult(FileImpl::make_file(path, root_));
        }
        else
        {
            // Ignore everything that's not a directory or file.
        }
    }
    catch (std::exception const&)
    {
        qf.reportException(StorageException());  // TODO
    }
    qf.reportFinished();
    return qf.future();
}

Root::SPtr RootImpl::make_root(QString const& identity, std::weak_ptr<Account> const& account)
{
    assert(account.lock());

    auto impl = new RootImpl(identity, account);
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
