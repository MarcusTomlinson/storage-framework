#pragma once

#include <unity/storage/qt/client/internal/AccountBase.h>

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
namespace local_client
{

class AccountImpl : public virtual AccountBase
{
public:
    AccountImpl(std::weak_ptr<Runtime> const& runtime,
                QString const& owner,
                QString const& owner_id,
                QString const& description);

    virtual QString owner() const override;
    virtual QString owner_id() const override;
    virtual QString description() const override;
    virtual QFuture<QVector<std::shared_ptr<Root>>> roots() override;

private:
    QString owner_;                           // Immutable
    QString owner_id_;                        // Immutable
    QString description_;                     // Immutable
    QVector<std::shared_ptr<Root>> roots_;    // Immutable
    std::atomic_bool copy_in_progress_;
};

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
