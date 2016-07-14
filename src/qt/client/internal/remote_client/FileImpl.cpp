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
#include <unity/storage/qt/client/internal/make_future.h>
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
    if (deleted_)
    {
        throw DeletedException();  // TODO
    }
    return 0;  // TODO
}

QFuture<shared_ptr<Uploader>> FileImpl::create_uploader(ConflictPolicy policy, int64_t size)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Uploader>>(DeletedException());
    }
    if (size < 0)
    {
        return make_exceptional_future<shared_ptr<Uploader>>(InvalidArgumentException());  // TODO
    }

    QString old_etag = policy == ConflictPolicy::overwrite ? "" : md_.etag;
    auto reply = provider().Update(md_.item_id, size, old_etag);

    auto process_reply = [this, size, old_etag](decltype(reply) const& reply, QFutureInterface<std::shared_ptr<Uploader>>& qf)
    {
        auto upload_id = reply.argumentAt<0>();
        auto fd = reply.argumentAt<1>();
        if (fd.fileDescriptor() < 0)
        {
            make_exceptional_future(qf, StorageException());  // TODO
        }
        else
        {
            auto uploader = UploaderImpl::make_uploader(upload_id, fd, size, old_etag, root_, provider());
            make_ready_future(qf, uploader);
        }
    };
    auto handler = new Handler<shared_ptr<Uploader>>(this, reply, process_reply);
    return handler->future();
}

QFuture<shared_ptr<Downloader>> FileImpl::create_downloader()
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Downloader>>(DeletedException());
    }

    auto process_create_downloader_reply = [this](QDBusPendingReply<QString, QDBusUnixFileDescriptor> const& reply,
                                                  QFutureInterface<std::shared_ptr<Downloader>>& qf)
    {
        auto download_id = reply.argumentAt<0>();
        auto fd = reply.argumentAt<1>();
        if (fd.fileDescriptor() < 0)
        {
            make_exceptional_future(qf, StorageException());  // TODO
        }
        else
        {
            auto file = dynamic_pointer_cast<File>(public_instance_.lock());
            auto downloader = DownloaderImpl::make_downloader(download_id, fd, file, provider());
            make_ready_future(qf, downloader);
        }
    };

    auto handler = new Handler<shared_ptr<Downloader>>(this,
                                                       provider().Download(md_.item_id),
                                                       process_create_downloader_reply);
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
