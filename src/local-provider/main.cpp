/*
 * Copyright (C) 2017 Canonical Ltd
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

#include <unity/storage/provider/Server.h>

#include "LocalProvider.h"

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;
using namespace unity::storage::provider;

int main(int argc, char* argv[])
{
    using namespace boost::filesystem;

    string const bus_name = "com.canonical.StorageFramework.Provider.Local";
    string const account_service_id = "";

    string progname = argv[0];

    try
    {
        progname = path(progname).filename().native();

        Server<LocalProvider> server(bus_name, account_service_id);
        server.init(argc, argv);
        server.run();
    }
    catch (std::exception const& e)
    {
        cerr << progname << ": " << e.what() << endl;
        return 1;
    }
}
