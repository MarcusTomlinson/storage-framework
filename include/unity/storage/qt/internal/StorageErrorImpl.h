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

#include <unity/storage/qt/StorageError.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class StorageErrorImpl
{
public:
    StorageErrorImpl();
    StorageErrorImpl(StorageError::Type type, QString const& msg);
    StorageErrorImpl(StorageError::Type type, QString const& msg, QString const& item_id);
    StorageErrorImpl(StorageError::Type type, QString const& msg, QString const& item_id, QString const& item_name);
    StorageErrorImpl(StorageError::Type type, QString const& msg, int error_code);
    StorageErrorImpl(StorageErrorImpl const&) = default;
    StorageErrorImpl(StorageErrorImpl&&) = default;
    ~StorageErrorImpl() = default;
    StorageErrorImpl& operator=(StorageErrorImpl const&) = default;
    StorageErrorImpl& operator=(StorageErrorImpl&&) = default;

    StorageError::Type type() const;
    QString name() const;
    QString message() const;
    QString errorString() const;

    QString itemId() const;
    QString itemName() const;
    int errorCode() const;

    // Factories to make things more convenient and ensure consistency.
    static StorageError local_comms_error(QString const& msg);
    static StorageError remote_comms_error(QString const& msg);
    static StorageError deleted_error(QString const& msg, QString const& item_id);
    static StorageError runtime_destroyed_error(QString const& msg);
    static StorageError not_exists_error(QString const& msg, QString const& key);
    static StorageError exists_error(QString const& msg, QString const& item_id, QString const& item_name);
    static StorageError conflict_error(QString const& msg);
    static StorageError permission_error(QString const& msg);
    static StorageError cancelled_error(QString const& msg);
    static StorageError logic_error(QString const& msg);
    static StorageError invalid_argument_error(QString const& msg);
    static StorageError resource_error(QString const& msg, int error_code);

private:
    StorageError::Type type_;
    QString name_;
    QString message_;
    QString item_id_;
    QString item_name_;
    int error_code_;
};


}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
