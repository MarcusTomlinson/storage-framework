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
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#pragma once

#include <unity/storage/visibility.h>

#include <memory>
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

namespace internal
{
class ServerImpl;
}

class ProviderBase;

class UNITY_STORAGE_EXPORT ServerBase
{
public:
    ServerBase(std::string const& bus_name, std::string const& account_service_id);
    virtual ~ServerBase();

    void init(int& argc, char** argv);
    int run();

protected:
    virtual std::shared_ptr<ProviderBase> make_provider() = 0;
private:
    std::unique_ptr<internal::ServerImpl> p_;

    friend class internal::ServerImpl;
};

template <typename T>
class Server : public ServerBase
{
public:
    using ServerBase::ServerBase;
protected:
    std::shared_ptr<ProviderBase> make_provider() override {
        return std::make_shared<T>();
    }
};

}
}
}
