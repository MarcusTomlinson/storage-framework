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

#include <unity/storage/qt/client/internal/local_client/RootImpl.h>

#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/make_future.h>
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
    return make_ready_future(QVector<Folder::SPtr>());  // For the root, we return an empty vector.
}

QVector<QString> RootImpl::parent_ids() const
{
    return QVector<QString>();  // For the root, we return an empty vector.
}

QFuture<void> RootImpl::delete_item()
{
    // Cannot delete root.
    return make_exceptional_future(StorageException());
}

QFuture<int64_t> RootImpl::free_space_bytes() const
{
    using namespace boost::filesystem;

    try
    {
        space_info si = space(identity_.toStdString());
        return make_ready_future<int64_t>(si.available);
    }
    catch (std::exception const&)
    {
        return make_exceptional_future<int64_t>(StorageException());  // TODO
    }
}

QFuture<int64_t> RootImpl::used_space_bytes() const
{
    using namespace boost::filesystem;

    try
    {
        space_info si = space(identity_.toStdString());
        return make_ready_future<int64_t>(si.capacity - si.available);
    }
    catch (std::exception const&)
    {
        return make_exceptional_future<int64_t>(StorageException());  // TODO
    }
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

        // Don't allow reserved files to be found.
        if (is_reserved_path(id_path))
        {
            throw NotExistException();
        }

        file_status s = status(id_path);
        QString path = QString::fromStdString(id_path.native());
        if (is_directory(s))
        {
            if (id_path == root_path)
            {
                return make_ready_future<Item::SPtr>(make_root(path, account_));
            }
            return make_ready_future<Item::SPtr>(make_folder(path, root_));
        }
        if (is_regular_file(s))
        {
            return make_ready_future<Item::SPtr>(FileImpl::make_file(path, root_));
        }
        throw StorageException();  // TODO
    }
    catch (std::exception const&)
    {
        return make_exceptional_future<Item::SPtr>(StorageException());  // TODO
    }
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
