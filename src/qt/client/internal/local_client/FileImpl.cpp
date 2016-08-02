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

#include <unity/storage/qt/client/internal/local_client/FileImpl.h>

#include <unity/storage/qt/client/Downloader.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/local_client/DownloaderImpl.h>
#include <unity/storage/qt/client/internal/local_client/filesystem_exception.h>
#include <unity/storage/qt/client/internal/local_client/UploaderImpl.h>
#include <unity/storage/qt/client/internal/make_future.h>
#include <unity/storage/qt/client/Uploader.h>

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

FileImpl::FileImpl(QString const& identity)
    : ItemBase(identity, ItemType::file)
    , FileBase(identity)
    , ItemImpl(identity, ItemType::file)
{
}

QString FileImpl::name() const
{
    lock_guard<decltype(mutex_)> guard(mutex_);

    if (deleted_)
    {
        throw deleted_ex("File::name()");
    }
    if (!get_root())
    {
        throw RuntimeDestroyedException("File::name()");
    }
    return name_;
}

int64_t FileImpl::size() const
{
    lock_guard<decltype(mutex_)> guard(mutex_);

    if (deleted_)
    {
        throw deleted_ex("File::size()");
    }
    if (!get_root())
    {
        throw RuntimeDestroyedException("File::size()");
    }

    try
    {
        boost::filesystem::path p = identity_.toStdString();
        return file_size(p);
    }
    catch (boost::filesystem::filesystem_error const& e)
    {
        throw_filesystem_exception(QString("File::size()"), e);
    }
    // LCOV_EXCL_START
    catch (std::exception const& e)
    {
        throw ResourceException(e.what(), 0);
    }
    // LCOV_EXCL_STOP
}

QFuture<Uploader::SPtr> FileImpl::create_uploader(ConflictPolicy policy, int64_t size)
{
    lock_guard<decltype(mutex_)> guard(mutex_);

    if (deleted_)
    {
        return internal::make_exceptional_future<Uploader::SPtr>(deleted_ex("File::create_uploader()"));
    }
    if (size < 0)
    {
        QString msg = "File::create_uploader(): size must be >= 0";
        return internal::make_exceptional_future<shared_ptr<Uploader>>(InvalidArgumentException(msg));
    }
    auto root = get_root();
    if (!root)
    {
        return internal::make_exceptional_future<Uploader::SPtr>(RuntimeDestroyedException("File::create_uploader()"));
    }

    auto file = dynamic_pointer_cast<File>(public_instance_.lock());
    assert(file);
    auto impl(new UploaderImpl(file, size, identity_, policy, root_));
    Uploader::SPtr ul(new Uploader(impl));
    return make_ready_future(ul);
}

QFuture<Downloader::SPtr> FileImpl::create_downloader()
{
    lock_guard<decltype(mutex_)> guard(mutex_);

    if (deleted_)
    {
        return internal::make_exceptional_future<Downloader::SPtr>(deleted_ex("File::create_downloader()"));
    }
    if (!get_root())
    {
        throw RuntimeDestroyedException("File::create_downloader()");
    }

    auto pi = public_instance_.lock();
    assert(pi);
    auto file_ptr = static_pointer_cast<File>(pi);
    auto impl = new DownloaderImpl(file_ptr);
    Downloader::SPtr dl(new Downloader(impl));
    return make_ready_future(dl);
}

File::SPtr FileImpl::make_file(QString const& identity, weak_ptr<Root> root)
{
    auto impl = new FileImpl(identity);
    File::SPtr file(new File(impl));
    impl->set_root(root);
    impl->set_public_instance(file);
    return file;
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
