#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/client/File.h>
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
    return 0;  // TODO
}

QFuture<shared_ptr<Uploader>> FileImpl::create_uploader(ConflictPolicy policy)
{
    QString old_etag = policy == ConflictPolicy::overwrite ? "" : md_.etag;
    ProviderInterface& prov = provider();
    auto handler = new UpdateHandler(prov.Update(md_.item_id, old_etag), old_etag, root_, prov);
    return handler->future();
}

QFuture<shared_ptr<Downloader>> FileImpl::create_downloader()
{
    return QFuture<shared_ptr<Downloader>>();
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
