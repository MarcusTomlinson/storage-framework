#include <unity/storage/qt/client/internal/local_client/RootImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/local_client/FileImpl.h>
#include <unity/storage/qt/client/Root.h>

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
namespace local_client
{

RootImpl::RootImpl(QString const& identity, weak_ptr<Account> const& account)
    : ItemBase(identity, ItemType::root)
    , FolderBase(identity, ItemType::root)
    , RootBase(identity, account)
    , ItemImpl(identity, ItemType::root)
    , FolderImpl(identity, ItemType::root)
{
    using namespace boost::filesystem;

    path id_path = path(identity.toStdString());
    if (!id_path.is_absolute())
    {
        QString msg = QString("Root: root path \"") + identity + "\" must be absolute";
        throw InvalidArgumentException(msg);
    }
    path can_path = canonical(id_path);
    auto id_len = std::distance(id_path.begin(), id_path.end());
    auto can_len = std::distance(can_path.begin(), can_path.end());
    if (id_len != can_len)
    {
        // identity denotes a weird path that we won't trust because
        // it might contain ".." or similar.
        QString msg = QString("Root: root path \"") + identity + "\" cannot contain \".\" or \"..\" components";
        throw InvalidArgumentException(msg);
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

QFuture<void> RootImpl::delete_item()
{
    // Cannot delete root.
    QFutureInterface<void> qf;
    qf.reportException(LogicException("Root::delete_item(): Cannot delete root folder"));
    qf.reportFinished();
    return qf.future();
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
    catch (std::exception const& e)
    {
        qf.reportException(ResourceException(QString("Root::free_space_bytes(): ") + e.what()));
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
    catch (std::exception const& e)
    {
        qf.reportException(ResourceException(QString("Root::used_space_bytes(): ") + e.what()));
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
            throw InvalidArgumentException("Root::get(): identity must be an absolute path");
        }

        // Make sure that native_identity is contained in or equal to the root path.
        id_path = canonical(id_path);
        auto root_path = path(root()->native_identity().toStdString());
        auto id_len = std::distance(id_path.begin(), id_path.end());
        auto root_len = std::distance(root_path.begin(), root_path.end());
        if (id_len < root_len || !std::equal(root_path.begin(), root_path.end(), id_path.begin()))
        {
            // native_identity can't possibly point at something below the root.
            QString msg = QString("Root::get(): identity \"") + native_identity + "\" points outside the root folder";
            throw InvalidArgumentException(msg);
        }

        // Don't allow reserved files to be found.
        if (is_reserved_path(id_path))
        {
            throw NotExistsException(QString("Root::get(): no such item: ") + native_identity, native_identity);
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
    catch (StorageException const& e)
    {
        qf.reportException(e);
    }
    catch (std::exception const& e)
    {
        qf.reportException(ResourceException(QString("Root::get(): ") + e.what()));
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

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
