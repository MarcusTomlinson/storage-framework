#include <unity/storage/qt/client/internal/FileImpl.h>

#include <unity/storage/qt/client/Downloader.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/Uploader.h>
#include <unity/storage/qt/client/internal/DownloaderImpl.h>

#include <boost/filesystem.hpp>
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

FileImpl::FileImpl(QString const& identity)
    : ItemImpl(identity, common::ItemType::file)
{
}

QFuture<void> FileImpl::destroy()
{
    if (destroyed_)
    {
        QFutureInterface<void> qf;
        qf.reportException(DestroyedException());
        return qf.future();
    }

    auto This = static_pointer_cast<FileImpl>(shared_from_this());  // Keep this file alive while the lambda is alive.
    auto destroy = [This]()
    {
        using namespace boost::filesystem;

        try
        {
            This->destroyed_ = true;
            remove(This->native_identity().toStdString());
        }
        catch (std::exception const&)
        {
            throw StorageException();  // TODO
        }
    };
    return QtConcurrent::run(destroy);
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
        return qf.future();
    }
    return QFuture<Uploader::SPtr>();  // TODO
}

QFuture<Downloader::SPtr> FileImpl::create_downloader()
{
    QFutureInterface<Downloader::SPtr> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        return qf.future();
    }

    auto pi = public_instance_.lock();
    assert(pi);
    auto file_ptr = static_pointer_cast<File>(pi);
    auto impl = new DownloaderImpl(file_ptr);  // TODO: missing params
    Downloader::SPtr dl(new Downloader(impl));
    qf.reportResult(dl);
    return qf.future();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
