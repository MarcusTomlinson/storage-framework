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

/**
\brief Base class to register a storage provider with the runtime.

\anchor extra-args This class allows you to register a provider that requires additional
parameters with the storage framework runtime, by overriding the
make_provider() method. (If your provider does not require constructor
arguments, use the predefined Server template instead.)

For example, here is how you can instantiate a provider that requires
a <code>some_value</code> constructor argument:

\code{.cpp}
class MyCloudProvider : public ProviderBase
{
public:
    MyCloudProvider(int some_value,
                    string const& bus_name,
                    string const& service_id)
        : ProviderBase(bus_name, service_id)
    {
    }
    // ...
};

class MyCloudServer : public ServerBase
{
public:
    MyCloudServer(int some_value,
                  string const& bus_name,
                  string const& account_service_id)
        : some_value_(some_value)
        , bus_name_(bus_name)
        , account_service_id_(account_service_id)
    {
    }

protected:
    shared_ptr<ProviderBase> make_provider() override
    {
        return make_shared<MyCloudProvider>(some_value, bus_name, account_service_id);
    }

private:
    int some_value_;
    string bus_name_;
    string account_service_id_;
};

int main(int argc, char* argv[])
{
    string const bus_name = "com.acme.StorageFramework.Provider.MyCloud";
    string const account_service_id = "storage-provider-mycloud";

    try
    {
        MyCloudServer server(99, bus_name, account_service_id);
        server.init(argc, argv);
        return server.run();
    }
    catch (std::exception const& e)
    {
        cerr << argv[0] << ": " << e.what() << endl;
        return 1;
    }
}
\endcode
*/

class UNITY_STORAGE_EXPORT ServerBase
{
public:
    /**
    \brief Constructs a server instance.
    \param bus_name The DBus name of the provider service on the session bus.
    \param account_service_id The service ID with which the provider is known to
    <a href="https://help.ubuntu.com/stable/ubuntu-help/accounts.html">Online Accounts</a>.
    */
    ServerBase(std::string const& bus_name, std::string const& account_service_id);
    virtual ~ServerBase();

    /**
    \brief Initializes the storage framework runtime.
    \param argc, argv The runtime passes these parameters through to
    <a href="http://doc.qt.io/qt-5/qcoreapplication.html">QCoreApplication</a>.
    \throws StorageException
    */
    void init(int& argc, char** argv);

    /**
    \brief Starts an event loop for the service.

    You <i>must</i> call init() before calling this method.
    \return In case of an error, run() returns non-zero status.
    */
    int run();

protected:
    /**
    \brief Factory method to instantiate a provider.
    The runtime calls this method to instantiate a provider as part
    of the init() method. You can override this method if you need
    to pass additional arguments to the constructor of your provider class.
    (see \ref extra-args "example code").
    \return A <code>shared_ptr</code> to the provider instance.
    */
    virtual std::shared_ptr<ProviderBase> make_provider() = 0;

private:
    std::unique_ptr<internal::ServerImpl> p_;

    friend class internal::ServerImpl;
};

/**
\brief Default ServerBase implementation for providers with a default constructor.

You can use this class to connect your provider class to the runtime, provided
the class has a default constructor. If you need to pass additional arguments
to the constructor, you must derive a class from ServerBase and override the
ServerBase::make_provider() factory method (see \ref extra-args "example code").
*/

template <typename T>
class Server : public ServerBase
{
public:
    using ServerBase::ServerBase;

protected:
    /**
    \brief Factory method to instantiate a provider with a default constructor.

    The runtime calls this method to create your provider instance of type T.
    \return A <code>shared_ptr</code> to the provider instance.
    */
    std::shared_ptr<ProviderBase> make_provider() override {
        return std::make_shared<T>();
    }
};

}
}
}
