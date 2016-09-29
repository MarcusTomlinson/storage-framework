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

#include <QObject>

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class VoidJobImpl;

}  // namespace internal

class StorageError;

class Q_DECL_EXPORT VoidJob final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::VoidJob::Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(unity::storage::qt::StorageError status READ error NOTIFY statusChanged FINAL)

public:
    virtual ~VoidJob();

    enum class Status { Loading, Finished, Error };
    Q_ENUMS(Status)

    bool isValid() const;
    Status status() const;
    StorageError error() const;

Q_SIGNALS:
    void statusChanged(unity::storage::qt::VoidJob::Status status) const;

private:
    VoidJob(std::unique_ptr<internal::VoidJobImpl> p);

    std::unique_ptr<internal::VoidJobImpl> const p_;

    friend class internal::VoidJobImpl;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::VoidJob::Status)
