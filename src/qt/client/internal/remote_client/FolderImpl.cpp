#include <unity/storage/qt/client/internal/remote_client/FolderImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>
#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>
#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>

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

// TODO: Move to header file?

namespace
{

class ListHandler : public QObject
{
    Q_OBJECT

public:
    ListHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                weak_ptr<Root> const& root,
                QString const& item_id,
                ProviderInterface& provider,
                QFutureInterface<QVector<Item::SPtr>> qf);

    QFuture<QVector<Item::SPtr>> future()
    {
        return qf_.future();
    }

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    weak_ptr<Root> root_;
    QString item_id_;
    ProviderInterface& provider_;
    QFutureInterface<QVector<Item::SPtr>> qf_;
};

ListHandler::ListHandler(QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                         weak_ptr<Root> const& root,
                         QString const& item_id,
                         ProviderInterface& provider,
                         QFutureInterface<QVector<Item::SPtr>> qf)
    : watcher_(reply, this)
    , root_(root)
    , item_id_(item_id)
    , provider_(provider)
    , qf_(qf)
{
    assert(root.lock());
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &ListHandler::finished);
}

void ListHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<QList<storage::internal::ItemMetadata>, QString> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    QVector<Item::SPtr> items;
    auto metadata = reply.argumentAt<0>();
    for (auto const& md : metadata)
    {
        switch (md.type)
        {
            case ItemType::file:
            {
                auto file = FileImpl::make_file(md, root_);
                items.append(file);
                break;
            }
            case ItemType::folder:
            {
                auto folder = FolderImpl::make_folder(md, root_);
                items.append(folder);
                break;
            }
            case ItemType::root:
            {
                // TODO: log impossible item type here
                continue;
            }
            default:
            {
                abort();  // LCOV_EXCL_LINE  // Impossible
            }
        }
    }
    qf_.reportResult(items, qf_.resultCount());

    QString token = reply.argumentAt<1>();
    if (token.isEmpty())
    {
        qf_.reportFinished();  // This was the last lot of results.
    }
    else
    {
        new ListHandler(provider_.List(item_id_, token), root_, item_id_, provider_, qf_);  // Request next lot.
    }
}

}  // namespace

QFuture<QVector<Item::SPtr>> FolderImpl::list() const
{
    QFutureInterface<QVector<Item::SPtr>> qf;
    qf.reportStarted();
    auto handler = new ListHandler(provider().List(md_.item_id, ""), root_, md_.item_id, provider(), qf);
    return handler->future();
}

class LookupHandler : public QObject
{
    Q_OBJECT

public:
    LookupHandler(QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                  weak_ptr<Root> const& root);

    QFuture<Item::SPtr> future()
    {
        return qf_.future();
    }

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<Item::SPtr> qf_;
    weak_ptr<Root> root_;
};

LookupHandler::LookupHandler(QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                            weak_ptr<Root> const& root)
    : watcher_(reply, this)
    , root_(root)
{
    assert(root.lock());
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &LookupHandler::finished);
    qf_.reportStarted();
}

void LookupHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<storage::internal::ItemMetadata> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    Item::SPtr item;
    auto metadata = reply.value();
    switch (metadata.type)
    {
        case ItemType::file:
        {
            item = FileImpl::make_file(metadata, root_);
            break;
        }
        case ItemType::folder:
        {
            item = FolderImpl::make_folder(metadata, root_);
            break;
        }
        case ItemType::root:
        {
            auto root_impl = dynamic_pointer_cast<RootImpl>(root_.lock()->p_);
            item = RootImpl::make_root(metadata, root_impl->account_);
            break;
        }
        default:
        {
            abort();  // LCOV_EXCL_LINE  // Impossible
        }
    }
    qf_.reportResult(item);
    qf_.reportFinished();
}

QFuture<Item::SPtr> FolderImpl::lookup(QString const& name) const
{
    QFutureInterface<Item::SPtr> qf;
    auto handler = new LookupHandler(provider().Lookup(md_.item_id, name), root_);
    return handler->future();
}

class CreateFolderHandler : public QObject
{
    Q_OBJECT

public:
    CreateFolderHandler(QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                        weak_ptr<Root> const& root);

    QFuture<shared_ptr<Folder>> future()
    {
        return qf_.future();
    }

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<shared_ptr<Folder>> qf_;
    weak_ptr<Root> root_;
};

CreateFolderHandler::CreateFolderHandler(QDBusPendingReply<storage::internal::ItemMetadata> const& reply,
                                         weak_ptr<Root> const& root)
    : watcher_(reply, this)
    , root_(root)
{
    assert(root.lock());
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &CreateFolderHandler::finished);
    qf_.reportStarted();
}

void CreateFolderHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<storage::internal::ItemMetadata> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    Item::SPtr item;
    auto metadata = reply.value();
    if (metadata.type != ItemType::folder)
    {
        qf_.reportException(StorageException());  // TODO need to log this as well, server error
    }
    else
    {
        qf_.reportResult(FolderImpl::make_folder(metadata, root_));
    }
    qf_.reportFinished();
}

QFuture<shared_ptr<Folder>> FolderImpl::create_folder(QString const& name)
{
    auto handler = new CreateFolderHandler(provider().CreateFolder(md_.item_id, name), root_);
    return handler->future();
}

class CreateFileHandler : public QObject
{
    Q_OBJECT

public:
    CreateFileHandler(QDBusPendingReply<QString, int> const& reply,
                      weak_ptr<Root> const& root);

    QFuture<shared_ptr<Uploader>> future()
    {
        return qf_.future();
    }

public Q_SLOTS:
    void finished(QDBusPendingCallWatcher* call);

private:
    QDBusPendingCallWatcher watcher_;
    QFutureInterface<shared_ptr<Uploader>> qf_;
    weak_ptr<Root> root_;
};

CreateFileHandler::CreateFileHandler(QDBusPendingReply<QString, int> const& reply,
                                     weak_ptr<Root> const& root)
    : watcher_(reply, this)
    , root_(root)
{
    assert(root.lock());
    connect(&watcher_, &QDBusPendingCallWatcher::finished, this, &CreateFileHandler::finished);
    qf_.reportStarted();
}

void CreateFileHandler::finished(QDBusPendingCallWatcher* call)
{
    deleteLater();

    QDBusPendingReply<QString, int> reply = *call;
    if (reply.isError())
    {
        qDebug() << reply.error().message();  // TODO, remove this
        qf_.reportException(StorageException());  // TODO
        qf_.reportFinished();
        return;
    }

    auto upload_id = reply.argumentAt<0>();
    auto fd = reply.argumentAt<1>();

    // TODO, incomplete

    qf_.reportFinished();
}

QFuture<shared_ptr<Uploader>> FolderImpl::create_file(QString const& name)
{
    auto handler = new CreateFileHandler(provider().CreateFile(md_.item_id, name, "application/octet-stream", false),
                                         root_);
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

#include "FolderImpl.moc"
