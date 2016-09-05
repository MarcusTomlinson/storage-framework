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

#include <unity/storage/qt/StorageError.h>
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

StorageError::StorageError()
    : p_(new internal::StorageErrorImpl)
{
}

StorageError::StorageError(StorageError const& other)
    : p_(new internal::StorageErrorImpl(*other.p_))
{
}

StorageError::StorageError(StorageError&&) = default;

StorageError::StorageError(unique_ptr<internal::StorageErrorImpl> p)
    : p_(move(p))
{
}

StorageError::~StorageError() = default;

StorageError& StorageError::operator=(StorageError const& other)
{
    *p_ = *other.p_;
    return *this;
}

StorageError& StorageError::operator=(StorageError&&) = default;

StorageError::Type StorageError::type() const
{
    return p_->type();
}

QString StorageError::name() const
{
    return p_->name();
}

QString StorageError::message() const
{
    return p_->message();
}

QString StorageError::errorString() const
{
    return p_->errorString();
}

QString StorageError::itemId() const
{
    return p_->itemId();
}

QString StorageError::itemName() const
{
    return p_->itemName();
}

int StorageError::errorCode() const
{
    return p_->errorCode();
}

}  // namespace qt
}  // namespace storage
}  // namespace unity
