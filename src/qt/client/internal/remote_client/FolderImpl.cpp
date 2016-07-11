#include <unity/storage/qt/client/internal/remote_client/FolderImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Folder.h>
#include <unity/storage/qt/client/internal/make_future.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/internal/remote_client/Handler.h>
#include <unity/storage/qt/client/internal/remote_client/UploaderImpl.h>

#include <QtConcurrent>

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

FolderImpl::FolderImpl(storage::internal::ItemMetadata const& md)
    : ItemBase(md.item_id, ItemType::folder)
    , FolderBase(md.item_id, ItemType::folder)
    , ItemImpl(md, ItemType::folder)
{
}

FolderImpl::FolderImpl(storage::internal::ItemMetadata const& md, ItemType type)
    : ItemBase(md.item_id, type)
    , FolderBase(md.item_id, type)
    , ItemImpl(md, type)
{
}

QFuture<QVector<shared_ptr<Item>>> FolderImpl::list() const
{
    if (deleted_)
    {
        return make_exceptional_future<QVector<shared_ptr<Item>>>(DeletedException());
    }

    auto reply = provider().List(md_.item_id, "");

    // Sorry for the mess, but we can't use auto for the lambda because it calls itself,
    // and the compiler can't deduce the type of the lambda while it's still parsing the lambda body.
    function<void(decltype(reply) const&, QFutureInterface<QVector<shared_ptr<Item>>>&)> process_reply
        = [this, &process_reply](decltype(reply) const& reply, QFutureInterface<QVector<shared_ptr<Item>>>& qf)
    {
        QVector<shared_ptr<Item>> items;
        auto metadata = reply.argumentAt<0>();
        for (auto const& md : metadata)
        {
            if (md.type == ItemType::root)
            {
                // TODO: log server error here
                continue;
            }
            items.append(ItemImpl::make_item(md, root_));
        }
        qf.reportResult(items, qf.resultCount());

        QString token = reply.argumentAt<1>();
        if (token.isEmpty())
        {
            qf.reportFinished();  // This was the last lot of results.
        }
        else
        {
            // Request next lot.
            auto next_reply = provider().List(md_.item_id, token);
            new Handler<QVector<shared_ptr<Item>>>(const_cast<FolderImpl*>(this), next_reply, process_reply);
        }
    };

    auto handler = new Handler<QVector<shared_ptr<Item>>>(const_cast<FolderImpl*>(this), reply, process_reply);
    return handler->future();
}

QFuture<QVector<shared_ptr<Item>>> FolderImpl::lookup(QString const& name) const
{
    if (deleted_)
    {
        return make_exceptional_future<QVector<shared_ptr<Item>>>(DeletedException());
    }

    auto reply = provider().Lookup(md_.item_id, name);
    auto process_reply = [this](decltype(reply) const& reply, QFutureInterface<QVector<shared_ptr<Item>>>& qf)
    {
        QVector<Item::SPtr> items;
        auto metadata = reply.value();
        for (auto const& md : metadata)
        {
            if (md.type == ItemType::root)
            {
                // TODO: log server error here
                continue;
            }
            items.append(ItemImpl::make_item(md, root_));
        }
        if (items.isEmpty())
        {
            make_exceptional_future(qf, StorageException());  // TODO
        }
        else
        {
            make_ready_future(qf, items);
        }
    };

    auto handler = new Handler<QVector<shared_ptr<Item>>>(const_cast<FolderImpl*>(this), reply, process_reply);
    return handler->future();
}

QFuture<shared_ptr<Folder>> FolderImpl::create_folder(QString const& name)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Folder>>(DeletedException());
    }

    auto reply = provider().CreateFolder(md_.item_id, name);
    auto process_reply = [this](decltype(reply) const& reply, QFutureInterface<shared_ptr<Folder>>& qf)
    {
        shared_ptr<Item> item;
        auto md = reply.value();
        if (md.type != ItemType::folder)
        {
            make_exceptional_future(qf, StorageException());  // TODO need to log this as well, server error
        }
        else
        {
            make_ready_future(qf, FolderImpl::make_folder(md, root_));
        }
    };

    auto handler = new Handler<shared_ptr<Folder>>(this, reply, process_reply);
    return handler->future();
}

QFuture<shared_ptr<Uploader>> FolderImpl::create_file(QString const& name)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Uploader>>(DeletedException());
    }

    auto reply = provider().CreateFile(md_.item_id, name, "application/octet-stream", false);
    auto process_reply = [this](decltype(reply) const& reply, QFutureInterface<shared_ptr<Uploader>>& qf)
    {
        auto upload_id = reply.argumentAt<0>();
        auto fd = reply.argumentAt<1>();
        auto uploader = UploaderImpl::make_uploader(upload_id, fd, "", root_, provider());
        make_ready_future(qf, uploader);
    };

    auto handler = new Handler<shared_ptr<Uploader>>(this, reply, process_reply);
    return handler->future();
}

shared_ptr<Folder> FolderImpl::make_folder(storage::internal::ItemMetadata const& md, weak_ptr<Root> root)
{
    assert(md.type == ItemType::folder);
    assert(root.lock());

    auto impl = new FolderImpl(md);
    shared_ptr<Folder> folder(new Folder(impl));
    impl->set_root(root);
    impl->set_public_instance(folder);
    return folder;
}

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
