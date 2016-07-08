#include <unity/storage/qt/client/internal/remote_client/AccountImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/internal/make_future.h>
#include <unity/storage/qt/client/internal/remote_client/Handler.h>
#include <unity/storage/qt/client/internal/remote_client/RootImpl.h>
#include <unity/storage/qt/client/internal/remote_client/RuntimeImpl.h>
#include <unity/storage/qt/client/Runtime.h>

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
namespace remote_client
{

static constexpr char BUS_NAME[] = "com.canonical.StorageFramework.Provider.ProviderTest";

AccountImpl::AccountImpl(weak_ptr<Runtime> const& runtime,
                         int account_id,
                         QString const& owner,
                         QString const& owner_id,
                         QString const& description)
    : AccountBase(runtime)
    , owner_(owner)
    , owner_id_(owner_id)
    , description_(description)
{
    auto rt_impl = dynamic_pointer_cast<RuntimeImpl>(runtime.lock()->p_);
    assert(rt_impl);
    QString bus_path = "/provider/" + QString::number(account_id);
    provider_.reset(new ProviderInterface(BUS_NAME, bus_path, rt_impl->connection()));
    if (!provider_->isValid())
    {
        throw LocalCommsException();  // TODO, details
    }
}

QString AccountImpl::owner() const
{
    return owner_;
}

QString AccountImpl::owner_id() const
{
    return owner_id_;
}

QString AccountImpl::description() const
{
    return description_;
}

QFuture<QVector<Root::SPtr>> AccountImpl::roots()
{
    auto process_roots_reply = [this](QDBusPendingReply<QList<storage::internal::ItemMetadata>> const& reply,
                                      QFutureInterface<QVector<Root::SPtr>>& qf)
    {
        QVector<shared_ptr<Root>> roots;
        auto metadata = reply.value();
        for (auto const& md : metadata)
        {
            if (md.type != ItemType::root)
            {
                // TODO: log impossible item type here
                continue;  // LCOV_EXCL_LINE
            }
            auto root = RootImpl::make_root(md, public_instance_);
            roots.append(root);
        }
        make_ready_future(qf, roots);
    };

    auto handler = new Handler<QVector<Root::SPtr>>(this, provider_->Roots(), process_roots_reply);
    return handler->future();
}

ProviderInterface& AccountImpl::provider()
{
    return *provider_;
}

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
