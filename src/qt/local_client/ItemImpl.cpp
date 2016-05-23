#include <unity/storage/qt/client/internal/ItemImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/FileImpl.h>
#include <unity/storage/qt/client/internal/FolderImpl.h>
#include <unity/storage/qt/client/Root.h>

#include <boost/filesystem.hpp>
#include <QtConcurrent>

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

ItemImpl::ItemImpl(QString const& identity, common::ItemType type)
    : type_(type)
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
    throw RuntimeDestroyedException();
}

common::ItemType ItemImpl::type() const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }
    return type_;
}

QFuture<QVariantMap> ItemImpl::get_metadata() const
{
    if (destroyed_)
    {
        QFutureInterface<QVariantMap> qf;
        qf.reportException(DestroyedException());  // TODO
        return qf.future();
    }

    return QFuture<QVariantMap>();  // TODO
}

QFuture<QDateTime> ItemImpl::last_modified_time() const
{
    if (destroyed_)
    {
        QFutureInterface<QDateTime> qf;
        qf.reportException(DestroyedException());  // TODO
        return qf.future();
    }

    auto This = static_pointer_cast<ItemImpl const>(shared_from_this());  // Keep this item alive while the lambda is alive.
    auto last_modified_time = [This]()
    {
        try
        {
            auto mtime = boost::filesystem::last_write_time(This->native_identity().toStdString());
            QDateTime dt;
            dt.setTime_t(mtime);
            return dt;
        }
        catch (std::exception const&)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(last_modified_time);
}

QFuture<shared_ptr<Item>> ItemImpl::copy(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    return QFuture<shared_ptr<Item>>();  // TODO
}

QFuture<shared_ptr<Item>> ItemImpl::move(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    if (destroyed_)
    {
        QFutureInterface<shared_ptr<Item>> qf;
        qf.reportException(DestroyedException());  // TODO
        return qf.future();
    }

    auto This = static_pointer_cast<ItemImpl>(shared_from_this());  // Keep this item alive while the lambda is alive.
    auto move = [This, new_parent, new_name]() -> Item::SPtr
    {
        using namespace boost::filesystem;

        try
        {
            if (new_parent->p_->destroyed_)
            {
                throw DestroyedException();  // TODO
            }
            if (This->type_ == common::ItemType::root)
            {
                throw StorageException();  // TODO
            }

            path target_path = new_parent->native_identity().toStdString();
            target_path += new_name.toStdString();
            if (exists(target_path))
            {
                throw StorageException();  // TODO
            }
            rename(This->native_identity().toStdString(), target_path);
            This->destroyed_ = true;
            if (This->type_ == common::ItemType::folder)
            {
                return This->make_folder(QString::fromStdString(target_path.native()), new_parent->p_->root_);
            }
            return This->make_file(QString::fromStdString(target_path.native()), new_parent->p_->root_);
        }
        catch (std::exception const&)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(move);
}

QFuture<QVector<Folder::SPtr>> ItemImpl::parents() const
{
    QFutureInterface<QVector<Folder::SPtr>> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        return qf.future();
    }

    using namespace boost::filesystem;

    // Doing this synchronously because we don't need to hit the file system.
    path p = native_identity().toStdString();
    QString parent = QString::fromStdString(p.parent_path().native());
    auto parent_folder = make_folder(parent, root_);
    QVector<Folder::SPtr> results;
    results.append(parent_folder);
    qf.reportResult(results);
    return qf.future();
}

void ItemImpl::set_root(weak_ptr<Root> p)
{
    assert(p.lock());
    root_ = p;
}

void ItemImpl::set_public_instance(weak_ptr<Item> p)
{
    assert(p.lock());
    public_instance_ = p;
}

weak_ptr<Item> ItemImpl::public_instance() const
{
    assert(public_instance_.lock());
    return public_instance_;
}

Folder::SPtr ItemImpl::make_folder(QString const& identity, weak_ptr<Root> root)
{
    auto impl = new FolderImpl(identity);
    Folder::SPtr folder(new Folder(impl));
    impl->set_root(root);
    impl->set_public_instance(folder);
    return folder;
}

File::SPtr ItemImpl::make_file(QString const& identity, weak_ptr<Root> root)
{
    auto impl = new FileImpl(identity);
    File::SPtr file(new File(impl));
    impl->set_root(root);
    impl->set_public_instance(file);
    return file;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
