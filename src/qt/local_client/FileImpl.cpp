#include <unity/storage/qt/client/internal/FileImpl.h>

#include <unity/storage/qt/client/Downloader.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/DownloaderImpl.h>
#include <unity/storage/qt/client/internal/UploaderImpl.h>
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

FileImpl::FileImpl(QString const& identity)
    : ItemImpl(identity, ItemType::file)
{
}

int64_t FileImpl::size() const
{
    if (destroyed_)
    {
        throw DestroyedException();
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
    QFutureInterface<Uploader::SPtr> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        qf.reportFinished();
        return qf.future();
    }

    try
    {
        auto pi = public_instance_.lock();
        assert(pi);
        auto file_ptr = static_pointer_cast<File>(pi);
        auto impl(new UploaderImpl(file_ptr, policy));
        Uploader::SPtr ul(new Uploader(impl));
        qf.reportResult(ul);
    }
    catch (std::exception const&)
    {
        qf.reportException(StorageException());  // TODO
    }
    qf.reportFinished();
    return qf.future();
}

QFuture<Downloader::SPtr> FileImpl::create_downloader()
{
    QFutureInterface<Downloader::SPtr> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        qf.reportFinished();
        return qf.future();
    }

    try
    {
        auto pi = public_instance_.lock();
        assert(pi);
        auto file_ptr = static_pointer_cast<File>(pi);
        auto impl = new DownloaderImpl(file_ptr);
        Downloader::SPtr dl(new Downloader(impl));
        qf.reportResult(dl);
    }
    catch (std::exception const&)
    {
        qf.reportException(StorageException());  // TODO
    }
    qf.reportFinished();
    return qf.future();
}

File::SPtr FileImpl::make_file(QString const& identity, weak_ptr<Root> root)
{
    auto impl = new FileImpl(identity);
    File::SPtr file(new File(impl));
    impl->set_root(root);
    impl->set_public_instance(file);
    return file;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
