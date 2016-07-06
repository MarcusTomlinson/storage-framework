#pragma once

#include <unity/storage/internal/ItemMetadata.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/internal/ItemBase.h>

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

class DeleteHandler;

class ItemImpl : public virtual ItemBase
{
public:
    ItemImpl(storage::internal::ItemMetadata const& md, ItemType type);

    virtual QString name() const override;
    virtual QVariantMap metadata() const override;
    virtual QDateTime last_modified_time() const override;
    virtual QFuture<std::shared_ptr<Item>> copy(std::shared_ptr<Folder> const& new_parent, QString const& new_name) override;
    virtual QFuture<std::shared_ptr<Item>> move(std::shared_ptr<Folder> const& new_parent, QString const& new_name) override;
    virtual QFuture<QVector<std::shared_ptr<Folder>>> parents() const override;
    virtual QVector<QString> parent_ids() const override;
    virtual QFuture<void> delete_item() override;
    virtual bool equal_to(ItemBase const& other) const noexcept override;

    ProviderInterface& provider() const noexcept;

protected:
    DeletedException deleted_ex(QString const& method) const noexcept;

    bool deleted_ = false;
    storage::internal::ItemMetadata md_;

    friend class DeleteHandler;
};

}  // namespace remote_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
