#include <unity/storage/qt/client/internal/remote_client/FileImpl.h>

#include <unity/storage/qt/client/Downloader.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/remote_client/DownloaderImpl.h>
#include <unity/storage/qt/client/internal/remote_client/UploaderImpl.h>
#include <unity/storage/qt/client/Uploader.h>

#include <boost/filesystem.hpp>

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

FileImpl::FileImpl(QString const& identity)
    : ItemBase(identity, ItemType::file)
    , FileBase(identity)
    , ItemImpl(identity, ItemType::file)
{
}

QString FileImpl::name() const
{
    return "";
}

int64_t FileImpl::size() const
{
    return 0;
}

QFuture<Uploader::SPtr> FileImpl::create_uploader(ConflictPolicy policy)
{
    return QFuture<Uploader::SPtr>();
}

QFuture<Downloader::SPtr> FileImpl::create_downloader()
{
    return QFuture<Downloader::SPtr>();
}

File::SPtr FileImpl::make_file(QString const& identity, weak_ptr<Root> root)
{
    auto impl = new FileImpl(identity);
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
