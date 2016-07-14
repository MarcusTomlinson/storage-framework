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

#include <unity/storage/qt/client/Exceptions.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

StorageException::StorageException() = default;

StorageException::~StorageException() = default;

StorageException* StorageException::clone() const
{
    return new StorageException(*this);
}

void StorageException::raise() const
{
    throw *this;
}

LocalCommsException::LocalCommsException() = default;

LocalCommsException::~LocalCommsException() = default;

LocalCommsException* LocalCommsException::clone() const
{
    return new LocalCommsException(*this);
}

void LocalCommsException::raise() const
{
    throw *this;
}

RemoteCommsException::RemoteCommsException() = default;

RemoteCommsException::~RemoteCommsException() = default;

RemoteCommsException* RemoteCommsException::clone() const
{
    return new RemoteCommsException(*this);
}

void RemoteCommsException::raise() const
{
    throw *this;
}

DeletedException::DeletedException() = default;

DeletedException::~DeletedException() = default;

DeletedException* DeletedException::clone() const
{
    return new DeletedException(*this);
}

void DeletedException::raise() const
{
    throw *this;
}

RuntimeDestroyedException::RuntimeDestroyedException() = default;

RuntimeDestroyedException::~RuntimeDestroyedException() = default;

RuntimeDestroyedException* RuntimeDestroyedException::clone() const
{
    return new RuntimeDestroyedException(*this);
}

void RuntimeDestroyedException::raise() const
{
    throw *this;
}

NotExistException::NotExistException() = default;

NotExistException::~NotExistException() = default;

NotExistException* NotExistException::clone() const
{
    return new NotExistException(*this);
}

void NotExistException::raise() const
{
    throw *this;
}

ConflictException::ConflictException() = default;

ConflictException::~ConflictException() = default;

ConflictException* ConflictException::clone() const
{
    return new ConflictException(*this);
}

void ConflictException::raise() const
{
    throw *this;
}

CancelledException::CancelledException() = default;

CancelledException::~CancelledException() = default;

CancelledException* CancelledException::clone() const
{
    return new CancelledException(*this);
}

void CancelledException::raise() const
{
    throw *this;
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
