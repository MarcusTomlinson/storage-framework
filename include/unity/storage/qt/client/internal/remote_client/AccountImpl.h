#pragma once

#include <unity/storage/qt/client/internal/AccountBase.h>

#include <QFuture>
#include <QVector>

#include <memory>

class ProviderInterface;

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

class AccountImpl : public QObject, public virtual AccountBase
{
public:
    AccountImpl(std::weak_ptr<Runtime> const& runtime,
                int account_id,
                QString const& owner,
                QString const& owner_id,
                QString const& description);

    virtual QString owner() const override;
    virtual QString owner_id() const override;
    virtual QString description() const override;
    virtual QFuture<QVector<std::shared_ptr<Root>>> roots() override;

    ProviderInterface& provider();

private:
    QString owner_;
    QString owner_id_;
    QString description_;
    std::shared_ptr<ProviderInterface> provider_;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity