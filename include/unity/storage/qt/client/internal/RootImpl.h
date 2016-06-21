#pragma once

#include <unity/storage/qt/client/Root.h>
#include <unity/storage/qt/client/internal/FolderImpl.h>

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

class RootImpl : public FolderImpl
{
public:
    RootImpl(QString const& identity, std::weak_ptr<Account> const& account);
    ~RootImpl() = default;
    RootImpl(RootImpl const&) = delete;
    RootImpl& operator=(RootImpl const&) = delete;

    virtual QString name() const override;
    virtual QFuture<QVector<Folder::SPtr>> parents() const override;
    virtual QVector<QString> parent_ids() const override;
    virtual QFuture<void> destroy() override;

    Account* account() const;
    QFuture<int64_t> free_space_bytes() const;
    QFuture<int64_t> used_space_bytes() const;
    QFuture<Item::SPtr> get(QString native_identity) const;

    static std::shared_ptr<Root> make_root(QString const& identity, std::weak_ptr<Account> const& account);

private:
    std::weak_ptr<Account> account_;

    friend class AccountImpl;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity