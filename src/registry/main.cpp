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

#include "registryadaptor.h"
#include <unity/storage/internal/TraceMessageHandler.h>
#include <unity/storage/registry/internal/qdbus-last-error-msg.h>
#include <unity/storage/registry/internal/RegistryAdaptor.h>
#include <unity/storage/registry/Registry.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusError>
#include <QFileInfo>
#pragma GCC diagnostic pop

using namespace unity::storage;
using namespace unity::storage::registry::internal;
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

        auto conn = QDBusConnection::sessionBus();

        registry::internal::RegistryAdaptor registry_adaptor(prog_name, conn);
        new ::RegistryAdaptor(&registry_adaptor);

        if (!conn.registerObject(registry::OBJECT_PATH, &registry_adaptor))
        {
            auto msg = last_error_msg(conn);
            throw runtime_error(string("Could not register object path ") + registry::OBJECT_PATH + msg.toStdString());
        }

        qDBusRegisterMetaType<unity::storage::internal::AccountDetails>();
        qDBusRegisterMetaType<QList<unity::storage::internal::AccountDetails>>();

        if (!conn.registerService(registry::BUS_NAME))
        {
            auto msg = last_error_msg(conn);
            throw runtime_error(string("Could not acquire DBus name ") + registry::BUS_NAME + msg.toStdString());
        }

        rc = app.exec();

        if (!conn.unregisterService(registry::BUS_NAME))
        {
            auto msg = last_error_msg(conn);
            throw runtime_error(string("Could not release DBus name ") + registry::BUS_NAME + msg.toStdString());
        }

        qDebug() << "Exiting";
    }
    catch (std::exception const& e)
    {
        qCritical().noquote() << QString(e.what());
    }

    return rc;
}
