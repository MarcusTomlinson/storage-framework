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

#include <unity/storage/qt/client/internal/ItemBase.h>

#include <unity/storage/qt/client/Account.h>
#include <unity/storage/qt/client/Exceptions.h>
#include <unity/storage/qt/client/Root.h>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{

ItemBase::ItemBase(QString const& identity, ItemType type)
    : identity_(identity)
    , type_(type)
{
    assert(!identity.isEmpty());
}

ItemBase::~ItemBase() = default;

QString ItemBase::native_identity() const
{
    return identity_;
}

ItemType ItemBase::type() const
{
    return type_;
}

Root* ItemBase::root() const
{
    if (auto r = root_.lock())
    {
        try
        {
            r->account();
        }
        catch (RuntimeDestroyedException const&)
        {
            throw RuntimeDestroyedException("Item::root()");
        }
        return r.get();
    }
    throw RuntimeDestroyedException("Item::root()");
}

void ItemBase::set_root(std::weak_ptr<Root> root)
{
    assert(root.lock());
    root_ = root;
}

void ItemBase::set_public_instance(std::weak_ptr<Item> p)
{
    assert(p.lock());
    public_instance_ = p;
}

shared_ptr<Root> ItemBase::get_root() const noexcept
{
    try
    {
        auto root = root_.lock();
        if (root)
        {
            root->account();  // Throws if either account or runtime has been destroyed.
            return root;
        }
    }
    catch (RuntimeDestroyedException const&)
    {
    }
    return nullptr;
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
