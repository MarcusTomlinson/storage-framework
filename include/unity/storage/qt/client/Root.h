#pragma once

#include <unity/storage/qt/client/Directory.h>

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

class RootImpl;

}

class Account;

/**
\brief Class that represents a root directory.
*/
class Root : public Directory
{
public:
    ~Root();
    Root(Root const&) = delete;
    Root& operator=(Root const&) = delete;
    Root(Root&&);
    Root& operator=(Root&&);

    typedef std::unique_ptr<Root> UPtr;

    /**
    \brief Returns the account for this item.
    */
    Account* account() const;

    QFuture<int64_t> free_space_bytes() const;
    QFuture<int64_t> used_space_bytes() const;

    QFuture<Item::UPtr> get(QString native_identity) const;

    // TODO: Do we need a method to get lots of things?

private:
    Root(internal::RootImpl*);

    std::unique_ptr<internal::RootImpl> p_;

    friend class internal::RootImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
