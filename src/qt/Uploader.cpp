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

#include <unity/storage/qt/Uploader.h>

#include <unity/storage/qt/internal/UploaderImpl.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{

Uploader::Uploader() = default;

Uploader::Uploader(unique_ptr<internal::UploaderImpl> p)
    : p_(move(p))
{
    assert(p_);
}

Uploader::~Uploader() = default;

bool Uploader::isValid() const
{
    return p_->isValid();
}

Uploader::Status Uploader::status() const
{
    return p_->status();
}

StorageError Uploader::error() const
{
    return p_->error();
}

Item::ConflictPolicy Uploader::policy() const
{
    return p_->policy();
}

qint64 Uploader::sizeInBytes() const
{
    return p_->sizeInBytes();
}

Item Uploader::item() const
{
    return p_->item();
}

void Uploader::finishUpload()
{
    p_->finishUpload();
}

void Uploader::cancel()
{
    p_->cancel();
}

}  // namespace qt
}  // namespace storage
}  // namespace unity
