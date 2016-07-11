#include <unity/storage/qt/client/internal/remote_client/FolderImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Folder.h>
#include <unity/storage/qt/client/internal/make_future.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/internal/remote_client/Handler.h>
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

    // Sorry for the mess, but we can't use auto for the lambda because it calls itself,
    // and the compiler can't deduce the type of the lambda while it's still parsing its body.
    std::function<void(QDBusPendingReply<QList<storage::internal::ItemMetadata>, QString> const&,
                       QFutureInterface<QVector<std::shared_ptr<Item>>>&)> process_list_reply
        = [this, process_list_reply](QDBusPendingReply<QList<storage::internal::ItemMetadata>, QString> const& reply,
                                     QFutureInterface<QVector<std::shared_ptr<Item>>>& qf)
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
            new Handler<QVector<shared_ptr<Item>>>(const_cast<FolderImpl*>(this),
                                                   provider().List(md_.item_id, token),
                                                   process_list_reply);
        }
    };

    auto handler = new Handler<QVector<shared_ptr<Item>>>(const_cast<FolderImpl*>(this),
                                                          provider().List(md_.item_id, ""),
                                                          process_list_reply);
    return handler->future();
}

QFuture<QVector<shared_ptr<Item>>> FolderImpl::lookup(QString const& name) const
{
    if (deleted_)
    {
        return make_exceptional_future<QVector<shared_ptr<Item>>>(DeletedException());
    }

    auto process_lookup_reply = [this](QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                                       QFutureInterface<QVector<std::shared_ptr<Item>>>& qf)
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

    auto handler = new Handler<QVector<shared_ptr<Item>>>(const_cast<FolderImpl*>(this),
                                                          provider().Lookup(md_.item_id, name),
                                                          process_lookup_reply);
    return handler->future();
}

QFuture<shared_ptr<Folder>> FolderImpl::create_folder(QString const& name)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Folder>>(DeletedException());
    }

    auto process_create_folder_reply = [this](QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                                              QFutureInterface<std::shared_ptr<Folder>>& qf)
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

    auto handler = new Handler<shared_ptr<Folder>>(this,
                                                   provider().CreateFolder(md_.item_id, name),
                                                   process_create_folder_reply);
    return handler->future();
}

QFuture<shared_ptr<Uploader>> FolderImpl::create_file(QString const& name)
{
    if (deleted_)
    {
        return make_exceptional_future<shared_ptr<Uploader>>(DeletedException());
    }

    auto process_create_file_reply = [this](QDBusPendingReply<QString, QDBusUnixFileDescriptor> const& reply,
                                            QFutureInterface<std::shared_ptr<Uploader>>& qf)
    {
        auto upload_id = reply.argumentAt<0>();
        auto fd = reply.argumentAt<1>();
        auto uploader = UploaderImpl::make_uploader(upload_id, fd, "", root_, provider());
        make_ready_future(qf, uploader);
    };

    auto handler = new Handler<shared_ptr<Uploader>>(this,
                                                     provider().CreateFile(md_.item_id,
                                                                           name,
                                                                           "application/octet-stream",
                                                                           false),
                                                     process_create_file_reply);
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
