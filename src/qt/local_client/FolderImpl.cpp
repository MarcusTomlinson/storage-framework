#include <unity/storage/qt/client/internal/FolderImpl.h>

#include <unity/storage/qt/client/Exceptions.h>

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

QFuture<QVector<Item::SPtr>> FolderImpl::list() const
{
    QFutureInterface<QVector<Item::SPtr>> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        return qf.future();
    }
    return QFuture<QVector<Item::SPtr>>();  // TODO
}

QFuture<QVector<Item::SPtr>> FolderImpl::lookup(QString const& name) const
{
    QFutureInterface<QVector<Item::SPtr>> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        return qf.future();
    }
    return QFuture<QVector<Item::SPtr>>();  // TODO
}

QFuture<QVector<Folder::SPtr>> FolderImpl::parents() const
{
    QFutureInterface<QVector<Folder::SPtr>> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        return qf.future();
    }
    return QFuture<QVector<Folder::SPtr>>();  // TODO
}

QFuture<Folder::SPtr> FolderImpl::create_folder(QString const& name)
{
    QFutureInterface<Folder::SPtr> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        return qf.future();
    }
    return QFuture<Folder::SPtr>();  // TODO
}

QFuture<shared_ptr<Uploader>> FolderImpl::create_file(QString const& name)
{
    QFutureInterface<shared_ptr<Uploader>> qf;
    if (destroyed_)
    {
        qf.reportException(DestroyedException());
        return qf.future();
    }
    return QFuture<shared_ptr<Uploader>>();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
