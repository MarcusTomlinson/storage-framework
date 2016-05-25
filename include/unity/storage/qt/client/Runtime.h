#pragma once

#include <unity/storage/visibility.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFuture>
#pragma GCC diagnostic pop

#include <memory>

namespace unity
{

namespace storage
{
namespace qt
{
namespace client
{

class Account;

namespace internal
{

class RuntimeImpl;

}  // namespace internal

/**
TODO
*/
class UNITY_STORAGE_EXPORT Runtime
{
public:
    /**
    \brief Destroys the runtime.

    The destructor implicitly calls shutdown().

    \warning Do not invoke methods on any other part of the API once the runtime is destroyed;
    doing so has undefined behavior.
    */
    ~Runtime();

    Runtime(Runtime const&) = delete;
    Runtime& operator=(Runtime const&) = delete;
    Runtime(Runtime&&);
    Runtime& operator=(Runtime&&);

    typedef std::shared_ptr<Runtime> SPtr;

    /**
    \brief Initializes the runtime.
    */
    static SPtr create();

    /**
    \brief Shuts down the runtime.

    This method shuts down the runtime. Calling shutdown() more than once is safe and does nothing.

    The destructor implicitly calls shutdown(). This method is provided mainly to permit logging of any
    errors that might arise during shut-down.
    \throws Various exceptions, depending on the error. TODO
    */
    void shutdown();

    QFuture<QVector<std::shared_ptr<Account>>> accounts();

private:
    Runtime(internal::RuntimeImpl* p);

    std::unique_ptr<internal::RuntimeImpl> p_;

    friend class RuntimeImpl;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
