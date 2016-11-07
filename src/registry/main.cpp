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

#include <unity/storage/internal/TraceMessageHandler.h>
#include <unity/storage/registry/internal/RegistryAdaptor.h>
#include <unity/storage/registry/Registry.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QFileInfo>

using namespace unity::storage;
using namespace std;

int main(int argc, char* argv[])
{
    auto const prog_name = QFileInfo(argv[0]).fileName();
    internal::TraceMessageHandler message_handler(prog_name);

    int rc = 1;
    try
    {
        qDebug() << "Initializing";

        QCoreApplication app(argc, argv);

        auto bus = QDBusConnection::sessionBus();

        registry::internal::RegistryInterface reg_intf;
        if (!bus.registerObject(registry::OBJECT_PATH, &reg_intf))
        {
            auto msg = bus.lastError().message().toStdString();
            if (!msg.empty())
            {
                msg = string(": ") + msg;
            }
            throw runtime_error(string("Could not register object path ") + registry::OBJECT_PATH + msg);
        }

        if (!bus.registerService(registry::BUS_NAME))
        {
            auto msg = bus.lastError().message().toStdString();
            if (!msg.empty())
            {
                msg = string(": ") + msg;
            }
            throw runtime_error(string("Could acquire DBus name ") + registry::BUS_NAME + msg);
        }

        rc = app.exec();

        if (!bus.unregisterService(registry::BUS_NAME))
        {
            auto msg = bus.lastError().message().toStdString();
            if (!msg.empty())
            {
                msg = string(": ") + msg;
            }
            throw runtime_error(string("Could not release DBus name ") + registry::BUS_NAME + msg);
        }

        qDebug() << "Exiting";
    }
    catch (std::exception const& e)
    {
        qCritical().noquote() << QString(e.what());
    }

    return rc;
}
