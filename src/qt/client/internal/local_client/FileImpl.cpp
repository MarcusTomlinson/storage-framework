#include <unity/storage/qt/client/internal/local_client/FileImpl.h>

#include <unity/storage/qt/client/Downloader.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/File.h>
#include <unity/storage/qt/client/internal/local_client/DownloaderImpl.h>
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
    lock_guard<mutex> guard(mutex_);

    if (deleted_)
    {
        throw DeletedException();
    }
    auto name = boost::filesystem::path(identity_.toStdString()).filename().native();
    return QString::fromStdString(name);
}

int64_t FileImpl::size() const
{
    lock_guard<mutex> guard(mutex_);

    if (deleted_)
    {
        throw DeletedException();
    }

    try
    {
        boost::filesystem::path p = identity_.toStdString();
        return file_size(p);
    }
    catch (std::exception const&)
    {
        throw StorageException();  // TODO
    }
}

QFuture<Uploader::SPtr> FileImpl::create_uploader(ConflictPolicy policy)
{
    lock_guard<mutex> guard(mutex_);

    if (deleted_)
    {
        return make_exceptional_future<Uploader::SPtr>(DeletedException());
    }

    try
    {
        auto file = dynamic_pointer_cast<File>(public_instance_.lock());
        assert(file);
        auto impl(new UploaderImpl(file, identity_, policy, root_));
        Uploader::SPtr ul(new Uploader(impl));
        return make_ready_future(ul);
    }
    catch (std::exception const&)
    {
        return make_exceptional_future<Uploader::SPtr>(StorageException());  // TODO
    }
}

QFuture<Downloader::SPtr> FileImpl::create_downloader()
{
    lock_guard<mutex> guard(mutex_);

    if (deleted_)
    {
        return make_exceptional_future<Downloader::SPtr>(DeletedException());
    }

    try
    {
        auto pi = public_instance_.lock();
        assert(pi);
        auto file_ptr = static_pointer_cast<File>(pi);
        auto impl = new DownloaderImpl(file_ptr);
        Downloader::SPtr dl(new Downloader(impl));
        return make_ready_future(dl);
    }
    catch (std::exception const&)
    {
        return make_exceptional_future<Downloader::SPtr>(StorageException());  // TODO
    }
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
