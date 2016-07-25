/*
 * Copyright (C) 2016 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/storage/qt/client/internal/local_client/FolderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/Folder.h>
#include <unity/storage/qt/client/Uploader.h>
#include <unity/storage/qt/client/internal/make_future.h>
#include <unity/storage/qt/client/internal/local_client/FileImpl.h>
#include <unity/storage/qt/client/internal/local_client/tmpfile-prefix.h>
#include <unity/storage/qt/client/internal/local_client/UploaderImpl.h>

#include <boost/algorithm/string/predicate.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtConcurrent>
#pragma GCC diagnostic pop

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

FolderImpl::FolderImpl(QString const& identity)
    : ItemBase(identity, ItemType::folder)
    , FolderBase(identity, ItemType::folder)
    , ItemImpl(identity, ItemType::folder)
{
}

FolderImpl::FolderImpl(QString const& identity, ItemType type)
    : ItemBase(identity, type)
    , FolderBase(identity, type)
    , ItemImpl(identity, type)
{
}

QString FolderImpl::name() const
{
    lock_guard<mutex> guard(mutex_);

    if (deleted_)
    {
        throw deleted_ex("Folder::name()");
    }
    return name_;
}

QFuture<QVector<Item::SPtr>> FolderImpl::list() const
{
    auto This = dynamic_pointer_cast<FolderImpl const>(shared_from_this());  // Keep this folder alive while the lambda is alive.
    auto list = [This]()
    {
        lock_guard<mutex> guard(This->mutex_);

        if (This->deleted_)
        {
            throw This->deleted_ex("Folder::list()");
        }
        auto root = This->get_root();
        if (!root)
        {
            throw RuntimeDestroyedException("Folder::list()");
        }

        try
        {
            using namespace boost::filesystem;

            QVector<Item::SPtr> results;
            for (directory_iterator it(This->native_identity().toStdString()); it != directory_iterator(); ++it)
            {
                auto dirent = *it;
                file_status s = dirent.status();
                if (is_reserved_path(dirent.path()))
                {
                    continue;  // Hide temp files that we create during copy() and move().
                }
                QString path = QString::fromStdString(dirent.path().native());
                if (is_directory(s))
                {
                    results.append(make_folder(path, root));
                }
                else if (is_regular_file(s))
                {
                    results.append(FileImpl::make_file(path, root));
                }
                else
                {
                    // Ignore everything that's not a directory or file.
                }
            }
            return results;
        }
        catch (boost::filesystem::filesystem_error const& e)
        {
            throw ResourceException(QString("Folder::list(): ") + e.what(), e.code().value());
        }
        catch (std::exception const& e)
        {
            throw ResourceException(QString("Folder::list(): ") + e.what(), errno);
        }
    };
    return QtConcurrent::run(list);
}

QFuture<QVector<Item::SPtr>> FolderImpl::lookup(QString const& name) const
{
    auto This = dynamic_pointer_cast<FolderImpl const>(shared_from_this());  // Keep this folder alive while the lambda is alive.
    auto lookup = [This, name]() -> QVector<Item::SPtr>
    {
        lock_guard<mutex> guard(This->mutex_);

        if (This->deleted_)
        {
            throw This->deleted_ex("Folder::lookup()");
        }
        auto root = This->get_root();
        if (!root)
        {
            throw RuntimeDestroyedException("Folder::lookup()");
        }

        try
        {
            using namespace boost::filesystem;

            path p = This->native_identity().toStdString();
            auto sanitized_name = sanitize(name, "Folder::lookup()");
            if (is_reserved_path(sanitized_name))
            {
                throw NotExistsException("Folder::lookup(): no such item: " + name, name);
            }
            p /= sanitized_name;
            file_status s = status(p);
            if (is_directory(s))
            {
                QVector<Item::SPtr> v;
                v.append(make_folder(QString::fromStdString(p.native()), root));
                return v;
            }
            if (is_regular_file(s))
            {
                QVector<Item::SPtr> v;
                v.append(FileImpl::make_file(QString::fromStdString(p.native()), root));
                return v;
            }
            throw NotExistsException("Folder::lookup(): no such item: " + name, name);
        }
        catch (StorageException const&)
        {
            throw;
        }
        catch (boost::filesystem::filesystem_error const& e)
        {
            if (e.code().value() == ENOENT)
            {
                throw NotExistsException("Folder::lookup(): no such item: " + name, name);
            }
            throw ResourceException(QString("Folder::lookup(): ") + e.what(), e.code().value());
        }
        catch (std::exception const& e)
        {
            throw ResourceException(QString("Folder::lookup(): ") + e.what(), errno);
        }
    };
    return QtConcurrent::run(lookup);
}

QFuture<Folder::SPtr> FolderImpl::create_folder(QString const& name)
{
    lock_guard<mutex> guard(mutex_);

    QFutureInterface<Folder::SPtr> qf;
    if (deleted_)
    {
        return make_exceptional_future<Folder::SPtr>(deleted_ex("Folder::create_folder()"));
    }
    auto root = get_root();
    if (!root)
    {
        return make_exceptional_future<Folder::SPtr>(RuntimeDestroyedException("Folder::create_folder()"));
    }

    try
    {
        using namespace boost::filesystem;

        path p = native_identity().toStdString();
        auto sanitized_name = sanitize(name, "Folder::create_folder()");
        if (is_reserved_path(sanitized_name))
        {
            QString msg = "Folder::create_folder(): names beginning with " + QString(TMPFILE_PREFIX) + " are reserved";
            return make_exceptional_future<Folder::SPtr>(InvalidArgumentException(msg));
        }
        p /= sanitized_name;
        create_directory(p);
        return make_ready_future(make_folder(QString::fromStdString(p.native()), root));
    }
    catch (StorageException const& e)
    {
        return make_exceptional_future<Folder::SPtr>(e);
    }
    catch (boost::filesystem::filesystem_error const& e)
    {
        return make_exceptional_future<Folder::SPtr>(ResourceException(QString("Folder::create_folder: ") + e.what(),
                                                                       e.code().value()));
    }
    catch (std::exception const& e)
    {
        return make_exceptional_future<Folder::SPtr>(ResourceException(QString("Folder::create_folder: ") + e.what(),
                                                                       errno));
    }
}

QFuture<shared_ptr<Uploader>> FolderImpl::create_file(QString const& name, int64_t size)
{
    unique_lock<mutex> guard(mutex_);

    if (deleted_)
    {
        return make_exceptional_future<Uploader::SPtr>(deleted_ex("Folder::create_file()"));
    }
    if (size < 0)
    {
        QString msg = "Folder::create_file(): size must be >= 0";
        return make_exceptional_future<shared_ptr<Uploader>>(InvalidArgumentException(msg));
    }
    auto root = get_root();
    if (!root)
    {
        return make_exceptional_future<Uploader::SPtr>(RuntimeDestroyedException("Folder::create_file()"));
    }

    try
    {
        using namespace boost::filesystem;

        path p = native_identity().toStdString();
        auto sanitized_name = sanitize(name, "Folder::create_file()");
        if (is_reserved_path(sanitized_name))
        {
            QString msg = "Folder::create_file(): names beginning with " + QString(TMPFILE_PREFIX) + " are reserved";
            return make_exceptional_future<Uploader::SPtr>(InvalidArgumentException(msg));
        }
        p /= sanitized_name;
        if (exists(p))
        {
            QString msg = "Folder::create_file(): item with name \"" + name + "\" exists already";
            return make_exceptional_future<Uploader::SPtr>(ExistsException(msg, native_identity(), name));
        }
        auto impl = new UploaderImpl(shared_ptr<File>(),
                                     size,
                                     QString::fromStdString(p.native()),
                                     ConflictPolicy::error_if_conflict,
                                     root);
        Uploader::SPtr uploader(new Uploader(impl));
        return make_ready_future(uploader);
    }
    catch (StorageException const& e)
    {
        return make_exceptional_future<Uploader::SPtr>(e);
    }
    catch (boost::filesystem::filesystem_error const& e)
    {
        return make_exceptional_future<Uploader::SPtr>(ResourceException(QString("Folder::create_file: ") + e.what(),
                                                                         e.code().value()));
    }
    catch (std::exception const& e)
    {
        return make_exceptional_future<Uploader::SPtr>(ResourceException(QString("Folder::create_file: ") + e.what(),
                                                                         errno));
    }
}

Folder::SPtr FolderImpl::make_folder(QString const& identity, weak_ptr<Root> root)
{
    auto impl = new FolderImpl(identity);
    Folder::SPtr folder(new Folder(impl));
    impl->set_root(root);
    impl->set_public_instance(folder);
    return folder;
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
