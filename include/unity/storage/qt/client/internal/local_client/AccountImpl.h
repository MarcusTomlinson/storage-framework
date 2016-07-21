/*
 * Copyright (C) 2016 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

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
};

}  // namespace local_client
}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
