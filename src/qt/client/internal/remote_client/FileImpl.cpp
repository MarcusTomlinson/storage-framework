#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/remote_client/DownloadHandler.h>
#include <unity/storage/qt/client/internal/remote_client/UpdateHandler.h>

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
        QFutureInterface<shared_ptr<Uploader>> qf;
        qf.reportException(DeletedException());  // TODO
        qf.reportFinished();
        return qf.future();
    }
    QString old_etag = policy == ConflictPolicy::overwrite ? "" : md_.etag;
    ProviderInterface& prov = provider();
    auto handler = new UpdateHandler(prov.Update(md_.item_id, old_etag), old_etag, root_, prov);
    return handler->future();
}

QFuture<shared_ptr<Downloader>> FileImpl::create_downloader()
{
    if (deleted_)
    {
        QFutureInterface<shared_ptr<Downloader>> qf;
        qf.reportException(DeletedException());  // TODO
        qf.reportFinished();
        return qf.future();
    }
    ProviderInterface& prov = provider();
    auto handler = new DownloadHandler(prov.Download(md_.item_id), dynamic_pointer_cast<File>(shared_from_this()),
                                       prov);
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
