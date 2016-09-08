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

#include <QMetaType>
#include <QString>

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class StorageErrorImpl;

}

class Q_DECL_EXPORT StorageError final
{
    Q_GADGET
    Q_PROPERTY(unity::storage::qt::StorageError::Type type READ type FINAL)
    Q_PROPERTY(QString name READ name FINAL)
    Q_PROPERTY(QString message READ message FINAL)
    Q_PROPERTY(QString errorString READ errorString FINAL)

    Q_PROPERTY(QString itemId READ itemId FINAL)
    Q_PROPERTY(QString itemName READ itemName FINAL)
    Q_PROPERTY(int errorCode READ errorCode FINAL)

public:
    StorageError();
    StorageError(StorageError const&);
    StorageError(StorageError&&);
    ~StorageError();
    StorageError& operator=(StorageError const&);
    StorageError& operator=(StorageError&&);

    enum Type
    {
        NoError, LocalCommsError, RemoteCommsError, Deleted, RuntimeDestroyed, NotExists,
        Exists, Conflict, PermissionDenied, Cancelled, LogicError, InvalidArgument, ResourceError,
        __LAST_STORAGE_ERROR
    };
    Q_ENUM(Type)

    Type type() const;
    QString name() const;
    QString message() const;
    QString errorString() const;

    QString itemId() const;
    QString itemName() const;
    int errorCode() const;

private:
    StorageError(std::unique_ptr<internal::StorageErrorImpl>);

    std::unique_ptr<internal::StorageErrorImpl> p_;

    friend class internal::StorageErrorImpl;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
