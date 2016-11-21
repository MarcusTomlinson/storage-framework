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

#include <unity/storage/qt/internal/AccountImpl.h>

#include "ProviderInterface.h"
#include <unity/storage/qt/Account.h>
#include <unity/storage/qt/internal/ItemImpl.h>
#include <unity/storage/qt/internal/ItemJobImpl.h>
#include <unity/storage/qt/internal/ItemListJobImpl.h>
#include <unity/storage/qt/internal/RuntimeImpl.h>
#include <unity/storage/qt/internal/StorageErrorImpl.h>
#include <unity/storage/qt/Runtime.h>

#include <boost/functional/hash.hpp>

#include <cassert>

using namespace std;

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

AccountImpl::AccountImpl()
    : is_valid_(false)
{
}

AccountImpl::AccountImpl(shared_ptr<RuntimeImpl> const& runtime_impl,
                         storage::internal::AccountDetails const& details)
    : is_valid_(true)
    , details_(details)
    , runtime_impl_(runtime_impl)
    , provider_(new ProviderInterface(details.busName, details.objectPath, runtime_impl->connection()))
{
    assert(!details.busName.isEmpty());
    assert(!details.objectPath.isEmpty());
}

QString AccountImpl::busName() const
{
    return is_valid_ ? details_.busName : "";
}

QString AccountImpl::objectPath() const
{
    return is_valid_ ? details_.objectPath : "";
}

QString AccountImpl::name() const
{
    return is_valid_ ? details_.name : "";
}

QString AccountImpl::providerId() const
{
    return is_valid_ ? details_.serviceId : "";
}

QString AccountImpl::providerName() const
{
    return is_valid_ ? details_.providerName : "";
}

QString AccountImpl::iconName() const
{
    return is_valid_ ? details_.iconName : "";
}

ItemListJob* AccountImpl::roots(QStringList const& keys) const
{
    QString const method = "Account::roots()";

    auto runtime = runtime_impl_.lock();
    if (!is_valid_)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot create job from invalid account");
        return ItemListJobImpl::make_job(e);
    }
    if (!runtime || !runtime->isValid())
    {
        auto e = StorageErrorImpl::runtime_destroyed_error(method + ": Runtime was destroyed previously");
        return ItemListJobImpl::make_job(e);
    }

    auto validate = [method](storage::internal::ItemMetadata const& md)
    {
        if (md.type != ItemType::root)
        {
            QString msg = method + ": provider returned non-root item type: " + QString::number(int(md.type));
            qCritical().noquote() << msg;
            throw StorageErrorImpl::local_comms_error(msg);
        }
    };

    auto reply = provider_->Roots(keys);
    auto This = const_pointer_cast<AccountImpl>(shared_from_this());
    return ItemListJobImpl::make_job(This, method, reply, validate);
}

ItemJob* AccountImpl::get(QString const& itemId, QStringList const& keys) const
{
    QString const method = "Account::get()";

    if (!is_valid_)
    {
        auto e = StorageErrorImpl::logic_error(method + ": cannot create job from invalid account");
        return ItemJobImpl::make_job(e);
    }
    auto runtime = runtime_impl_.lock();
    if (!runtime || !runtime->isValid())
    {
        auto e = StorageErrorImpl::runtime_destroyed_error(method + ": Runtime was destroyed previously");
        return ItemJobImpl::make_job(e);
    }

    auto validate = [](storage::internal::ItemMetadata const&)
    {
    };

    auto reply = provider_->Metadata(itemId, keys);
    auto This = const_pointer_cast<AccountImpl>(shared_from_this());
    return ItemJobImpl::make_job(This, method, reply, validate);
}

bool AccountImpl::operator==(AccountImpl const& other) const
{
    if (is_valid_)
    {
        return other.is_valid_ && details_ == other.details_;
    }
    return !other.is_valid_;
}

bool AccountImpl::operator!=(AccountImpl const& other) const
{
    return !operator==(other);
}

bool AccountImpl::operator<(AccountImpl const& other) const
{
    if (!is_valid_)
    {
        return other.is_valid_;
    }
    if (is_valid_ && !other.is_valid_)
    {
        return false;
    }
    return details_ < other.details_;
}

bool AccountImpl::operator<=(AccountImpl const& other) const
{
    return operator<(other) || operator==(other);
}

bool AccountImpl::operator>(AccountImpl const& other) const
{
    return !operator<=(other);
}

bool AccountImpl::operator>=(AccountImpl const& other) const
{
    return !operator<(other);
}

shared_ptr<RuntimeImpl> AccountImpl::runtime_impl() const
{
    return runtime_impl_.lock();
}

shared_ptr<ProviderInterface> AccountImpl::provider() const
{
    return provider_;
}

size_t AccountImpl::hash() const
{
    if (!is_valid_)
    {
        return 0;
    }
    size_t hash = details_.id;
    boost::hash_combine(hash, qHash(details_.serviceId));
    boost::hash_combine(hash, qHash(details_.name));
    return hash;
}

Account AccountImpl::make_account(shared_ptr<RuntimeImpl> const& runtime,
                                  storage::internal::AccountDetails const& details)
{
    shared_ptr<AccountImpl> p(new AccountImpl(runtime, details));
    return Account(p);
}

}  // namespace internal
}  // namespace qt
}  // namespace storage
}  // namespace unity
