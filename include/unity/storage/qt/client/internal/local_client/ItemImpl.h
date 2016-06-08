#pragma once

#include <unity/storage/common.h>
#include <unity/storage/qt/client/internal/ItemBase.h>

#include <boost/filesystem.hpp>

#include <mutex>

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

class ItemImpl : public virtual ItemBase
{
public:
    ItemImpl(QString const& identity, ItemType type);
    virtual ~ItemImpl();

    virtual QString name() const override;
    virtual QVariantMap metadata() const override;
    virtual QDateTime last_modified_time() const override;
    virtual QFuture<std::shared_ptr<Item>> copy(std::shared_ptr<Folder> const& new_parent, QString const& new_name) override;
    virtual QFuture<std::shared_ptr<Item>> move(std::shared_ptr<Folder> const& new_parent, QString const& new_name) override;
    virtual QFuture<QVector<std::shared_ptr<Folder>>> parents() const override;
    virtual QVector<QString> parent_ids() const override;
    virtual QFuture<void> destroy() override;
    virtual bool equal_to(ItemBase const& other) const noexcept override;

    QDateTime get_modified_time();
    void update_modified_time();

    std::unique_lock<std::mutex> get_lock();

protected:
    static boost::filesystem::path sanitize(QString const& name);

    bool destroyed_;
    QString name_;
    QDateTime modified_time_;
    QVariantMap metadata_;
    static std::mutex mutex_;
};

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
