#pragma once

#include <unity/storage/visibility.h>

#include <QDateTime>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QMap>
#pragma GCC diagnostic pop
#include <QVariant>

#include <memory>

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

class MetadataImpl;

namespace local_client
{

class ItemImpl;

}  // namespace local_client

namespace remote_client
{

class ItemImpl;

}  // namespace remote_client
}  // namespace internal

typedef QMap<QString, QVariant> MetadataMap;

/**
This class provides access to metadata that may not be available from all providers.
If a provider does not support a particular metadata item, the corresponding accessor
returns a Qt type in an invalid state (such as an invalid `QDateTime`). For metadata
items that do not have a built-in "invalid" value (such as `int`), the corresponding
accessor returns a null `QVariant`.

The native_metadata() method returns provider-specific metadata. Note that if you use
the data returned by this method, your application will be dependent on one or more
specific providers and their versions. Unless you know that your application will
only be used with a specific provider, you must provide reasonable fallback behavior
in case a particular metadata item is not available.
*/

class UNITY_STORAGE_EXPORT Metadata final
{
public:
    Metadata();
    Metadata(Metadata const&);
    Metadata& operator=(Metadata const&);
    Metadata(Metadata&&);
    Metadata& operator=(Metadata&&);

    /**
    \brief Returns the time at which an item was created.
    \return If a provider does not support this method, the returned `QDateTime`'s `isValid()`
    method returns false.
    */
    QDateTime creation_time() const;

    /**
    \brief Returns provider-specific metadata.

    The contents of the returned map depend on the actual provider. This method is provided
    to allow applications to use provider-specific features that may not be
    supported by all providers.
    \return The returned map may be empty if a provider does not support this feature. If a provider
    supports it, the following keys are guaranteed to be present:
        - `native_provider_id` (string)
          A string that identifies the provider, such as "mCloud".
        - `native_provider_version` (string)
          A string that provides a version identifier.
    \warn Unless you know that your application will only be used with a specific provider,
    avoid using this method. If you do use provider-specific data, ensure reasonable fallback
    behavior for your application if it encounters a different provider that does not provide
    a particular metadata item.
    // TODO: document where to find the list of metadata items for each concrete provider.
    */
    MetadataMap native_metadata() const;

private:
    Metadata(std::shared_ptr<internal::MetadataImpl> const&) UNITY_STORAGE_HIDDEN;

    std::shared_ptr<internal::MetadataImpl> p_;

    friend class internal::local_client::ItemImpl;
    friend class internal::remote_client::ItemImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
