#include <unity/storage/qt/client/internal/MetadataImpl.h>

#include <unity/storage/internal/MetadataKeys.h>

using namespace unity::storage::internal;
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

QDateTime MetadataImpl::creation_time() const
{
    auto it = md_.find(CREATION_TIME);
    if (it != md_.end())
    {
        return it->toDateTime();
    }
    return QDateTime();
}

MetadataImpl& MetadataImpl::creation_time(QDateTime const& time)
{
    md_.insert(CREATION_TIME, QVariant(time));
    return *this;
}

MetadataMap MetadataImpl::native_metadata() const
{
    return MetadataMap();
}

MetadataImpl& MetadataImpl::native_metadata(MetadataMap const& md)
{
    md_ = md;
    return *this;
}

MetadataImpl& MetadataImpl::native_metadata(MetadataMap const&& md)
{
    md_ = md;
    return *this;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
