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

#include <unity/storage/qt/client/File.h>

#include <unity/storage/qt/client/internal/FileBase.h>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

using namespace internal;

File::File(FileBase* p)
    : Item(p)
{
}

File::~File() = default;

int64_t File::size() const
{
    return dynamic_cast<FileBase*>(p_.get())->size();
}

QFuture<shared_ptr<Uploader>> File::create_uploader(ConflictPolicy policy)
{
    return dynamic_cast<FileBase*>(p_.get())->create_uploader(policy);
}

QFuture<shared_ptr<Downloader>> File::create_downloader()
{
    return dynamic_cast<FileBase*>(p_.get())->create_downloader();
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
