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

#include <unity/storage/provider/Server.h>
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/internal/ServerImpl.h>


using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{

ServerBase::ServerBase(std::string const& bus_name, std::string const& account_service_id)
    : p_(new internal::ServerImpl(this, bus_name, account_service_id))
{
}

ServerBase::~ServerBase() = default;

void ServerBase::init(int& argc, char** argv)
{
    p_->init(argc, argv);
}

int ServerBase::run()
{
    return p_->run();
}

}
}
}
