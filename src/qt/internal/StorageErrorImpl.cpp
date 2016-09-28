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

#include <unity/storage/qt/internal/StorageErrorImpl.h>

#include <cassert>
#include <memory>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

namespace
{

static char const * const ERROR_NAMES[StorageError::__LAST_STORAGE_ERROR] =
{
    "NoError", "LocalCommsError", "RemoteCommsError", "Deleted", "RuntimeDestroyed", "NotExists",
    "Exists", "Conflict", "PermissionDenied", "Cancelled", "LogicError", "InvalidArgument", "ResourceError"
};

}  // namespace

StorageErrorImpl::StorageErrorImpl()
    : type_(StorageError::Type::NoError)
    , name_(ERROR_NAMES[type_])
    , message_("No error")
    , error_code_(0)
{
}

StorageErrorImpl::StorageErrorImpl(StorageError::Type type, QString const& msg)
    : type_(type)
    , name_(ERROR_NAMES[type_])
    , message_(msg)
    , error_code_(0)
{
    assert(   type == StorageError::Type::LocalCommsError
           || type == StorageError::Type::RemoteCommsError
           || type == StorageError::Type::RuntimeDestroyed
           || type == StorageError::Type::Conflict
           || type == StorageError::Type::PermissionDenied
           || type == StorageError::Type::Cancelled
           || type == StorageError::Type::LogicError
           || type == StorageError::Type::InvalidArgument);
    assert(!msg.isEmpty());
}

StorageErrorImpl::StorageErrorImpl(StorageError::Type type, QString const& msg, QString const& key)
    : type_(type)
    , name_(ERROR_NAMES[type_])
    , message_(msg)
    , error_code_(0)
{
    assert(   type == StorageError::Type::Deleted
           || type == StorageError::Type::NotExists);
    assert(!msg.isEmpty());

    item_id_ = key;
    if (type == StorageError::Type::NotExists)
    {
        item_name_ = key;
    }
}

StorageErrorImpl::StorageErrorImpl(StorageError::Type type,
                                   QString const& msg,
                                   QString const& item_id,
                                   QString const& item_name)
    : type_(type)
    , name_(ERROR_NAMES[type_])
    , message_(msg)
    , item_id_(item_id)
    , item_name_(item_name)
    , error_code_(0)
{
    assert(type == StorageError::Type::Exists);
    assert(!msg.isEmpty());
    assert(!item_id.isEmpty());
    assert(!item_name.isEmpty());
}

StorageErrorImpl::StorageErrorImpl(StorageError::Type type, QString const& msg, int error_code)
    : type_(type)
    , name_(ERROR_NAMES[type_])
    , message_(msg)
    , error_code_(error_code)
{
    assert(type == StorageError::Type::ResourceError);
    assert(!msg.isEmpty());
}

StorageError::Type StorageErrorImpl::type() const
{
    return type_;
}

QString StorageErrorImpl::name() const
{
    return name_;
}

QString StorageErrorImpl::message() const
{
    return message_;
}

QString StorageErrorImpl::errorString() const
{
    return name_ + ": " + message_;
}

QString StorageErrorImpl::itemId() const
{
    return item_id_;
}

QString StorageErrorImpl::itemName() const
{
    return item_name_;
}

int StorageErrorImpl::errorCode() const
{
    return error_code_;
}

StorageError StorageErrorImpl::make_error(StorageError::Type type, QString const& msg)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(type, msg));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::local_comms_error(QString const& msg)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::LocalCommsError, msg));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::remote_comms_error(QString const& msg)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::RemoteCommsError, msg));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::deleted_error(QString const& msg, QString const& item_id)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::Deleted, msg, item_id));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::runtime_destroyed_error(QString const& msg)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::RuntimeDestroyed, msg));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::not_exists_error(QString const& msg, QString const& key)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::NotExists, msg, key));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::exists_error(QString const& msg, QString const& item_id, QString const& item_name)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::Exists, msg, item_id, item_name));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::conflict_error(QString const& msg)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::Conflict, msg));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::permission_error(QString const& msg)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::PermissionDenied, msg));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::cancelled_error(QString const& msg)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::Cancelled, msg));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::logic_error(QString const& msg)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::LogicError, msg));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::invalid_argument_error(QString const& msg)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::InvalidArgument, msg));
    return StorageError(move(p));
}

StorageError StorageErrorImpl::resource_error(QString const& msg, int error_code)
{
    unique_ptr<StorageErrorImpl> p(new StorageErrorImpl(StorageError::Type::ResourceError, msg, error_code));
    return StorageError(move(p));
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
