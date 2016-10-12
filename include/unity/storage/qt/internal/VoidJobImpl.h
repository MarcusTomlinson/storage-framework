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

#include <unity/storage/qt/VoidJob.h>

#include <unity/storage/qt/StorageError.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDBusPendingReply>
#pragma GCC diagnostic pop

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class ItemImpl;

class VoidJobImpl : public QObject
{
    Q_OBJECT
public:
    virtual ~VoidJobImpl() = default;

    bool isValid() const;
    VoidJob::Status status() const;
    StorageError error() const;

    static VoidJob* make_job(std::shared_ptr<ItemImpl> const& item_impl,
                             QString const& method,
                             QDBusPendingReply<void>& reply);
    static VoidJob* make_job(StorageError const& e);

private:
    VoidJobImpl(std::shared_ptr<ItemImpl> const& item_impl,
                QString const& method,
                QDBusPendingReply<void>& reply);
    VoidJobImpl(StorageError const& e);

    VoidJob* public_instance_;
    VoidJob::Status status_;
    StorageError error_;
    QString method_;
    std::shared_ptr<ItemImpl> item_impl_;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
