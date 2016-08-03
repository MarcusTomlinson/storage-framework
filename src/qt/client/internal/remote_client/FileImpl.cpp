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

#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/remote_client/Handler.h>
#include <unity/storage/qt/client/internal/remote_client/DownloaderImpl.h>
#include <unity/storage/qt/client/internal/remote_client/UploaderImpl.h>

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
namespace remote_client
{

FileImpl::FileImpl(storage::internal::ItemMetadata const& md)
    : ItemBase(md.item_id, ItemType::file)
    , FileBase(md.item_id)
    , ItemImpl(md, ItemType::file)
{
}

int64_t FileImpl::size() const
{
    throw_if_destroyed("File::size()");
    return 0;  // TODO
}

QFuture<shared_ptr<Uploader>> FileImpl::create_uploader(ConflictPolicy policy, int64_t size)
{
    try
    {
        throw_if_destroyed("File::create_uploader()()");
    }
    catch (StorageException const& e)
    {
        return make_exceptional_future<shared_ptr<Uploader>>(e);
    }
    if (size < 0)
    {
        QString msg = "File::create_uploader(): size must be >= 0";
        return make_exceptional_future<shared_ptr<Uploader>>(InvalidArgumentException(msg));
    }

    QString old_etag = policy == ConflictPolicy::overwrite ? "" : md_.etag;
    auto prov = provider();
    auto reply = prov->Update(md_.item_id, size, old_etag);

    auto process_reply = [this, size, old_etag, prov](decltype(reply) const& reply,
                                                      QFutureInterface<std::shared_ptr<Uploader>>& qf)
    {
        auto root = get_root();
        if (!root)
        {
            make_exceptional_future(qf, RuntimeDestroyedException("File::create_uploader()"));
            return;
        }

        auto upload_id = reply.argumentAt<0>();
        auto fd = reply.argumentAt<1>();
        if (fd.fileDescriptor() < 0)
        {
            // TODO: log server error here
            QString msg = "File::create_uploader(): impossible file descriptor returned by server: "
                          + QString::number(fd.fileDescriptor());
            make_exceptional_future(qf, LocalCommsException(msg));
            return;
        }
        auto uploader = UploaderImpl::make_uploader(upload_id, fd, size, old_etag, root, prov);
        make_ready_future(qf, uploader);
    };
    auto handler = new Handler<shared_ptr<Uploader>>(this, reply, process_reply);
    return handler->future();
}

QFuture<shared_ptr<Downloader>> FileImpl::create_downloader()
{
    try
    {
        throw_if_destroyed("File::create_downloader()()");
    }
    catch (StorageException const& e)
    {
        return make_exceptional_future<shared_ptr<Downloader>>(e);
    }

    auto prov = provider();
    auto reply = prov->Download(md_.item_id);

    auto process_reply = [this, prov](QDBusPendingReply<QString, QDBusUnixFileDescriptor> const& reply,
                                      QFutureInterface<std::shared_ptr<Downloader>>& qf)
    {
        try
        {
            throw_if_destroyed("File::create_downloader()");
        }
        catch (StorageException const& e)
        {
            make_exceptional_future(qf, RuntimeDestroyedException("File::create_uploader()"));
        }

        auto download_id = reply.argumentAt<0>();
        auto fd = reply.argumentAt<1>();
        if (fd.fileDescriptor() < 0)
        {
            // TODO: log server error here
            QString msg = "File::create_downloader(): impossible file descriptor returned by server: "
                          + QString::number(fd.fileDescriptor());
            make_exceptional_future(qf, LocalCommsException(msg));
            return;
        }
        auto file = dynamic_pointer_cast<File>(public_instance_.lock());
        // TODO: provider may not be around anymore if the runtime was destroyed.
        auto downloader = DownloaderImpl::make_downloader(download_id, fd, file, prov);
        make_ready_future(qf, downloader);
    };

    auto handler = new Handler<shared_ptr<Downloader>>(this, reply, process_reply);
    return handler->future();
}

File::SPtr FileImpl::make_file(storage::internal::ItemMetadata const& md, weak_ptr<Root> root)
{
    auto impl = new FileImpl(md);
    File::SPtr file(new File(impl));
    impl->set_root(root);
    impl->set_public_instance(file);
    return file;
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
