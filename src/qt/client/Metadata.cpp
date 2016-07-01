#include <unity/storage/qt/client/Metadata.h>

#include <unity/storage/qt/client/internal/MetadataImpl.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

Metadata::Metadata()
    : p_(new internal::MetadataImpl)
{
}

Metadata::Metadata(shared_ptr<internal::MetadataImpl> const& p)
    : p_(p)
{
    assert(p);
}

Metadata::Metadata(Metadata const& other)
    : p_(new internal::MetadataImpl(*(other.p_)))
{
}

Metadata& Metadata::operator=(Metadata const& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    p_.reset(new internal::MetadataImpl(*(rhs.p_)));
    return *this;
}

Metadata::Metadata(Metadata&&) = default;

Metadata& Metadata::operator=(Metadata&&) = default;

QDateTime Metadata::creation_time() const
{
    return p_->creation_time();
}

MetadataMap Metadata::native_metadata() const
{
    return p_->native_metadata();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
