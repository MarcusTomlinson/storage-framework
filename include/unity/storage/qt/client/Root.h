#pragma once

#include <unity/storage/qt/client/Folder.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Account;
class Item;

namespace internal
{

class RootBase;

namespace local_client
{

class RootImpl;

}  // namespace local_client

namespace remote_client
{

class RootImpl;

}  // namespace remote_client
}  // namespace internal

/**
\brief Class that represents a root folder.
*/
class UNITY_STORAGE_EXPORT Root final : public Folder
{
public:
    virtual ~Root();
    Root(Root const&) = delete;
    Root& operator=(Root const&) = delete;
    Root(Root&&);
    Root& operator=(Root&&);

    typedef std::shared_ptr<Root> SPtr;

    /**
    \brief Returns the account for this root.
    */
    Account* account() const;

    QFuture<int64_t> free_space_bytes() const;
    QFuture<int64_t> used_space_bytes() const;

    QFuture<Item::SPtr> get(QString native_identity) const;

    // TODO: Do we need a method to get lots of things?

private:
    Root(internal::RootBase*);

    friend class internal::local_client::RootImpl;
    friend class internal::remote_client::RootImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
