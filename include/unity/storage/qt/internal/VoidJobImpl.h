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

#include <QDBusPendingReply>

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

    static VoidJob* make_void_job(std::shared_ptr<ItemImpl> const& item,
                                  QString const& method,
                                  QDBusPendingReply<void> const& reply);
    static VoidJob* make_void_job(StorageError const& e);

private:
    VoidJobImpl(std::shared_ptr<ItemImpl> const& item,
                QString const& method,
                QDBusPendingReply<void> const& reply);
    VoidJobImpl(StorageError const& e);

    VoidJob* public_instance_;
    VoidJob::Status status_;
    StorageError error_;
    QString method_;
    std::shared_ptr<ItemImpl> item_;
};

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
