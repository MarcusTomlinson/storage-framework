#include <unity/storage/qt/client/internal/FolderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/FileImpl.h>

#include <boost/filesystem.hpp>
#include <QtConcurrent>

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
    : ItemImpl(identity, common::ItemType::folder)
{
}

FolderImpl::FolderImpl(QString const& identity, common::ItemType type)
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
                   results.append(This->make_folder(path, This->root_));
                }
                else if (is_regular_file(s))
                {
                   results.append(This->make_file(path, This->root_));
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

QFuture<Item::SPtr> FolderImpl::lookup(QString const& name) const
{
    if (destroyed_)
    {
        QFutureInterface<Item::SPtr> qf;
        qf.reportException(DestroyedException());
        return qf.future();
    }

    auto This = static_pointer_cast<FolderImpl const>(shared_from_this());  // Keep this folder alive while the lambda is alive.
    auto lookup = [This, name]() -> Item::SPtr
    {
        try
        {
            using namespace boost::filesystem;

            path p = This->native_identity().toStdString();
            p += name.toStdString();
            file_status s = status(p);
            if (is_directory(s))
            {
                return This->make_folder(QString::fromStdString(p.native()), This->root_);
            }
            if (is_regular_file(s))
            {
                return This->make_file(QString::fromStdString(p.native()), This->root_);
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
        return qf.future();
    }

    auto This = static_pointer_cast<FolderImpl>(shared_from_this());  // Keep this folder alive while the lambda is alive.
    auto create_folder = [This, name]()
    {
        using namespace boost::filesystem;

        try
        {
            path p = This->native_identity().toStdString();
            p += name.toStdString();
            create_directory(p);
            return This->make_folder(QString::fromStdString(p.native()), This->root_);
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
        return qf.future();
    }
    return QFuture<shared_ptr<Uploader>>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
