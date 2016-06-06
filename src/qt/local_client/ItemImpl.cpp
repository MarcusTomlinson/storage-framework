#include <unity/storage/qt/client/internal/ItemImpl.h>

#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/AccountImpl.h>
#include <unity/storage/qt/client/internal/FileImpl.h>
#include <unity/storage/qt/client/internal/RootImpl.h>

#include <boost/filesystem.hpp>
#include <QtConcurrent>

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

ItemImpl::ItemImpl(QString const& identity, ItemType type)
    : destroyed_(false)
    , type_(type)
{
    assert(!identity.isEmpty());
    auto path = boost::filesystem::canonical(identity.toStdString());
    identity_ = QString::fromStdString(path.native());
    name_ = QString::fromStdString(path.filename().native());
    update_modified_time();
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

ItemType ItemImpl::type() const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }
    return type_;
}

QVariantMap ItemImpl::metadata() const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }
    return metadata_;
}

QDateTime ItemImpl::last_modified_time() const
{
    if (destroyed_)
    {
        throw DestroyedException();  // TODO
    }

    lock_guard<mutex> lock(mutex_);
    return modified_time_;
}

void ItemImpl::update_modified_time()
{
    lock_guard<mutex> lock(mutex_);
    auto mtime = boost::filesystem::last_write_time(native_identity().toStdString());
    modified_time_ = QDateTime::fromTime_t(mtime);
}

namespace
{

using namespace boost::filesystem;

void copy_recursively(path const& source, path const& target)
{
    auto s = status(source);
    if (is_regular_file(s))
    {
        copy_file(source, target);
        return;
    }
    else if (is_directory(s))
    {
        copy_directory(source, target);  // Poorly named in boost; this creates the target dir without recursion
        for (directory_iterator it(source); it != directory_iterator(); ++it)
        {
            path source_entry = it->path();
            path target_entry = target;
            target_entry /= source_entry.filename();
            copy_recursively(source_entry, target_entry);
        }
    }
    else
    {
        // Ignore everything that's not a directory or file.
    }
}

}  // namespace

QFuture<shared_ptr<Item>> ItemImpl::copy(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    using namespace boost::filesystem;

    if (destroyed_)
    {
        QFutureInterface<shared_ptr<Item>> qf;
        qf.reportException(DestroyedException());  // TODO
        qf.reportFinished();
        return qf.future();
    }

    auto This = static_pointer_cast<ItemImpl>(shared_from_this());  // Keep this item alive while the lambda is alive.
    auto copy = [This, new_parent, new_name]() -> Item::SPtr
    {
        try
        {
            if (new_parent->p_->destroyed_)
            {
                throw DestroyedException();  // TODO
            }
            if (This->root()->account() != new_parent->root()->account())
            {
                // Can't do cross-account copy.
                throw StorageException();  // TODO
            }

            path source_path = This->native_identity().toStdString();
            path parent_path = new_parent->native_identity().toStdString();
            path target_path = parent_path;
            path sanitized_name = sanitize(new_name);
            target_path /= sanitized_name;

            if (This->type_ == ItemType::file)
            {
                copy_file(source_path, target_path);
                return FileImpl::make_file(QString::fromStdString(target_path.native()), new_parent->p_->root_);
            }

            // Allow only one recursive copy per account at a time, otherwise source and destination
            // sub-trees for recursive copies could potentially overlap, creating chaos due to iterator invalidation.
            auto root_impl = static_pointer_cast<Root>(This->root_.lock());
            if (!root_impl)
            {
                throw RuntimeDestroyedException();
            }

            auto guard = root_impl->account()->p_->get_copy_guard();

            if (exists(target_path))
            {
                throw StorageException();  // TODO
            }

            // For recursive copy, we create a temporary directory in lieu of target_path and recursively copy
            // everything into the temporary directory. This ensures that we don't invalidate directory iterators
            // by creating things while we are iterating, potentially getting trapped in an infinite loop.
            path tmp_path = canonical(parent_path);
            tmp_path /= unique_path(".%%%%-%%%%-%%%%-%%%%");
            create_directories(tmp_path);
            for (directory_iterator it(source_path); it != directory_iterator(); ++it)
            {
                if (tmp_path.compare(canonical(it->path())) == 0)
                {
                    continue;  // Don't recurse into the temporary directory
                }
                file_status s = it->status();
                if (is_directory(s) || is_regular_file(s))
                {
                    path source_entry = it->path();
                    path target_entry = tmp_path;
                    target_entry /= source_entry.filename();
                    copy_recursively(source_entry, target_entry);
                }
            }
            rename(tmp_path, target_path);
            return FolderImpl::make_folder(QString::fromStdString(target_path.native()), new_parent->p_->root_);
        }
        catch (std::exception const&)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(copy);
}

QFuture<shared_ptr<Item>> ItemImpl::move(shared_ptr<Folder> const& new_parent, QString const& new_name)
{
    if (destroyed_)
    {
        QFutureInterface<shared_ptr<Item>> qf;
        qf.reportException(DestroyedException());  // TODO
        qf.reportFinished();
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
            if (This->root()->account() != new_parent->root()->account())
            {
                // Can't do cross-account move.
                throw StorageException();  // TODO
            }
            if (This->type_ == ItemType::root)
            {
                // Can't move a root.
                throw StorageException();  // TODO
            }

            path target_path = new_parent->native_identity().toStdString();
            target_path /= sanitize(new_name);
            if (exists(target_path))
            {
                throw StorageException();  // TODO
            }
            rename(This->native_identity().toStdString(), target_path);
            This->destroyed_ = true;
            if (This->type_ == ItemType::folder)
            {
                return FolderImpl::make_folder(QString::fromStdString(target_path.native()), new_parent->p_->root_);
            }
            return FileImpl::make_file(QString::fromStdString(target_path.native()), new_parent->p_->root_);
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
        qf.reportFinished();
        return qf.future();
    }

    Root::SPtr root = root_.lock();
    if (!root)
    {
        throw RuntimeDestroyedException();
    }

    using namespace boost::filesystem;

    // We do this synchronously because we don't need to hit the file system.
    path p = native_identity().toStdString();
    QString parent_path = QString::fromStdString(p.parent_path().native());

    QVector<Folder::SPtr> results;
    if (parent_path != root->p_->identity_)
    {
        results.append(FolderImpl::make_folder(parent_path, root_));
    }
    else
    {
        results.append(root);
    }
    qf.reportResult(results);
    qf.reportFinished();
    return qf.future();
}

QVector<QString> ItemImpl::parent_ids() const
{
    if (destroyed_)
    {
        throw DestroyedException();
    }

    using namespace boost::filesystem;

    // We do this synchronously because we don't need to hit the file system.
    path p = native_identity().toStdString();
    QString parent_path = QString::fromStdString(p.parent_path().native());

    QVector<QString> results;
    results.append(parent_path);
    return results;
}

QFuture<void> ItemImpl::destroy()
{
    if (destroyed_)
    {
        QFutureInterface<void> qf;
        qf.reportException(DestroyedException());
        qf.reportFinished();
        return qf.future();
    }

    auto This = shared_from_this();  // Keep this item alive while the lambda is alive.
    auto destroy = [This]()
    {
        using namespace boost::filesystem;

        try
        {
            remove_all(This->identity_.toStdString());
            This->destroyed_ = true;
        }
        catch (std::exception const& e)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(destroy);
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

bool ItemImpl::operator==(ItemImpl const& other) const noexcept
{
    if (destroyed_ || other.destroyed_)
    {
        return false;
    }
    return identity_ == other.identity_;
}

// Throw if name contains more than one path component.
// Otherwise, return the relative path for the name.
// This is to make sure that calling, say, create_file()
// with a name such as "../../whatever" cannot lead
// outside the root.

boost::filesystem::path ItemImpl::sanitize(QString const& name)
{
    using namespace boost::filesystem;

    path p = name.toStdString();
    if (!p.parent_path().empty())
    {
        // name contains more than one component.
        throw StorageException();  // TODO.
    }
    path filename = p.filename();
    if (filename.empty() || filename == "." || filename == "..")
    {
        // Not an allowable file name.
        throw StorageException();  // TODO.
    }
    return p;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
