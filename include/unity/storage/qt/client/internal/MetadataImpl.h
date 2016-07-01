#pragma once

#include <unity/storage/qt/client/Metadata.h>

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

class MetadataImpl
{
public:
    MetadataImpl() = default;
    MetadataImpl(MetadataImpl const&) = default;
    MetadataImpl& operator=(MetadataImpl const&) = default;
    MetadataImpl(MetadataImpl&&) = default;
    MetadataImpl& operator=(MetadataImpl&&) = default;

    QDateTime creation_time() const;
    MetadataImpl& creation_time(QDateTime const& time);

    MetadataMap native_metadata() const;
    MetadataImpl& native_metadata(MetadataMap const& md);
    MetadataImpl& native_metadata(MetadataMap const&& md);

private:
    QMap<QString, QVariant> md_;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
