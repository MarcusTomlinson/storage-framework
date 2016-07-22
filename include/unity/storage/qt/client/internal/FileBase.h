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

#include <unity/storage/qt/client/internal/ItemBase.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

class Downloader;
class File;
class Uploader;

namespace internal
{

class FileBase : public virtual ItemBase
{
public:
    FileBase(QString const& identity);

    virtual int64_t size() const = 0;
    virtual QFuture<std::shared_ptr<Uploader>> create_uploader(ConflictPolicy policy, int64_t size) = 0;
    virtual QFuture<std::shared_ptr<Downloader>> create_downloader() = 0;
};

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
