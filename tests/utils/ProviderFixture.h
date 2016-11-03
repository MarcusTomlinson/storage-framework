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

#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/testing/TestServer.h>
#include <utils/DBusEnvironment.h>
#include <utils/ProviderClient.h>

#include <gtest/gtest.h>
#include <QDBusConnection>

#include <memory>

class ProviderFixture : public ::testing::Test
{
public:
    ProviderFixture();
    virtual ~ProviderFixture();

    virtual void SetUp() override;
    virtual void TearDown() override;

    QDBusConnection const& connection() const;
    void set_provider(std::unique_ptr<unity::storage::provider::ProviderBase>&& provider);
    void wait_for(QDBusPendingCall const& call);
    QString bus_name() const;
    QString object_path() const;

protected:
    std::unique_ptr<DBusEnvironment> dbus_;
private:
    class ServiceThread;
    std::unique_ptr<ServiceThread> service_thread_;
};
