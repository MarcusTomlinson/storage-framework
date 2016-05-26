#include <unity/storage/qt/client/internal/FolderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/FileImpl.h>

#include <boost/filesystem.hpp>
#include <QtConcurrent>

#include <fcntl.h>

#include <QDebug>  // TODO: remove this

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

FolderImpl::FolderImpl(QString const& identity)
    : ItemImpl(identity, ItemType::folder)
{
}

FolderImpl::FolderImpl(QString const& identity, ItemType type)
    : ItemImpl(identity, type)
{
}

FolderImpl::~FolderImpl() = default;

QFuture<void> FolderImpl::destroy()
{
    if (destroyed_)
    {
        QFutureInterface<QVector<Item::SPtr>> qf;
        qf.reportException(DestroyedException());
        qf.reportFinished();
        return qf.future();
    }

    auto This = static_pointer_cast<FolderImpl>(shared_from_this());  // Keep this folder alive while the lambda is alive.
    auto destroy = [This]()
    {
        using namespace boost::filesystem;

        try
        {
            This->destroyed_ = true;
            remove_all(This->native_identity().toStdString());
        }
        catch (std::exception const&)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(destroy);
}

QFuture<QVector<Item::SPtr>> FolderImpl::list() const
{
    if (destroyed_)
    {
        QFutureInterface<QVector<Item::SPtr>> qf;
        qf.reportException(DestroyedException());
        qf.reportFinished();
        return qf.future();
    }

    auto This = static_pointer_cast<FolderImpl const>(shared_from_this());  // Keep this folder alive while the lambda is alive.
    auto list = [This]()
    {
        try
        {
            using namespace boost::filesystem;

            QVector<Item::SPtr> results;
            for (directory_iterator it(This->native_identity().toStdString()); it != directory_iterator(); ++it)
            {
                auto dirent = *it;
                file_status s = dirent.status();
                QString path = QString::fromStdString(dirent.path().native());
                if (is_directory(s))
                {
                    results.append(make_folder(path, This->root_));
                }
                else if (is_regular_file(s))
                {
                    results.append(FileImpl::make_file(path, This->root_));
                }
                else
                {
                    // Ignore everything that's not a directory or file.
                }
            }
            return results;
        }
        catch (std::exception const&)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(list);
}

namespace
{

// Throw if name contains more than one path component.
// Otherwise, return the relative path for the name.
// This is to make sure that calling, say, create_file()
// with a name such as "../../whatever" cannot lead
// outside the root.

boost::filesystem::path sanitize(QString const& name)
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

}  // namespace

QFuture<Item::SPtr> FolderImpl::lookup(QString const& name) const
{
    if (destroyed_)
    {
        QFutureInterface<Item::SPtr> qf;
        qf.reportException(DestroyedException());
        qf.reportFinished();
        return qf.future();
    }

    auto This = static_pointer_cast<FolderImpl const>(shared_from_this());  // Keep this folder alive while the lambda is alive.
    auto lookup = [This, name]() -> Item::SPtr
    {
        try
        {
            using namespace boost::filesystem;

            path p = This->native_identity().toStdString();
            p += sanitize(name);
            file_status s = status(p);
            if (is_directory(s))
            {
                return make_folder(QString::fromStdString(p.native()), This->root_);
            }
            if (is_regular_file(s))
            {
                return FileImpl::make_file(QString::fromStdString(p.native()), This->root_);
            }
            throw NotExistException();  // TODO
        }
        catch (std::exception const&)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(lookup);
}

QFuture<Folder::SPtr> FolderImpl::create_folder(QString const& name)
{
    if (destroyed_)
    {
        QFutureInterface<Folder::SPtr> qf;
        qf.reportException(DestroyedException());
        qf.reportFinished();
        return qf.future();
    }

    auto This = static_pointer_cast<FolderImpl>(shared_from_this());  // Keep this folder alive while the lambda is alive.
    auto create_folder = [This, name]()
    {
        using namespace boost::filesystem;

        try
        {
            path p = This->native_identity().toStdString();
            p += sanitize(name);
            create_directory(p);
            return make_folder(QString::fromStdString(p.native()), This->root_);
        }
        catch (std::exception const&)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(create_folder);
}

QFuture<shared_ptr<Uploader>> FolderImpl::create_file(QString const& name)
{
    if (destroyed_)
    {
        QFutureInterface<shared_ptr<Uploader>> qf;
        qf.reportException(DestroyedException());
        qf.reportFinished();
        return qf.future();
    }

    auto This = static_pointer_cast<FolderImpl>(shared_from_this());  // Keep this folder alive while the lambda is alive.
    auto create_file = [This, name]()
    {
        using namespace boost::filesystem;

        try
        {
            path p = This->native_identity().toStdString();
            p += sanitize(name);
            int fd = open(p.native().c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);  // Fails if file already exists.
            if (fd == -1)
            {
                throw StorageException();  // TODO
            }
            if (close(fd) == -1)
            {
                throw StorageException();  // TODO
            }
            auto file = FileImpl::make_file(QString::fromStdString(p.native()), This->root_);
            return file->create_uploader(ConflictPolicy::overwrite);
        }
        catch (std::exception const&)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(create_file);
}

Folder::SPtr FolderImpl::make_folder(QString const& identity, weak_ptr<Root> root)
{
    auto impl = new FolderImpl(identity);
    Folder::SPtr folder(new Folder(impl));
    impl->set_root(root);
    impl->set_public_instance(folder);
    return folder;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
