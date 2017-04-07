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

#pragma once

#include <unity/storage/qt/Account.h>
#include <unity/storage/qt/StorageError.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDBusConnection>
#pragma GCC diagnostic pop

#include <memory>

class QDBusConnection;

namespace unity
{
namespace storage
{
/**
\brief This namespace contains the client-side API.
*/
namespace qt
{
namespace internal
{

class RuntimeImpl;

}  // namespace internal

class AccountsJob;

/**
\brief Class to access to the storage framework runtime.

Before you can do anything with the storage framework API, you must instantiate the Runtime.

*/
class Q_DECL_EXPORT Runtime : public QObject
{
    Q_OBJECT

    /**
    \see \link isValid() const isValid()\endlink
    */
    Q_PROPERTY(bool isValid READ isValid FINAL)

    /**
    \see \link error() const error()\endlink
    */
    Q_PROPERTY(unity::storage::qt::StorageError error READ error FINAL)

    /**
    \see \link connection() const connection()\endlink
    */
    Q_PROPERTY(QDBusConnection connection READ connection CONSTANT FINAL)

public:
    /**
    \brief Initializes the storage framework runtime.

    You can interact with the storage framework API only while a Runtime instance exists.
    You must create a Runtime instance before doing anything
    else, and you must not interact with anything obtained via that runtime once it
    is destroyed.
    (Any calls to other instances obtained via this runtime after it has been destroyed
    or after shutdown() was called fail with StorageError::RuntimeDestroyed.)

    You can create more than one Runtime instance, but should generally not have any need to do so. (Each
    instance uses a separate DBus connection to access providers.)
    \param parent The runtime's owner object.
    */
    Runtime(QObject* parent = nullptr);

    /**
    \brief Initializes the storage framework runtime for a specific DBus connection.

    This constructor allows you to provide a separate DBus connection that the runtime will
    use to access DBus services. This constructor is provided for testing the storage framework
    API itself; you should not have any need to use it for a normal application.
    \param bus The DBus connection to use for accessing DBus services.
    \param parent The runtime's owner object.
    */
    Runtime(QDBusConnection const& bus, QObject* parent = nullptr);

    /**
    \brief Destroys the storage framework runtime.

    The destructor finalizes the runtime, that is, disconnects all signals and tears down the DBus connection.
    Any calls to other instances obtained via this runtime after it has been destroyed
    or after shutdown() was called fail with StorageError::RuntimeDestroyed.
    */
    virtual ~Runtime();

    /**
    \brief Checks whether this runtime was successfully constructed.

    If any errors were encountered during initialization, isValid() returns <code>false</code>
    and youu can obtain further details about the cause of the error via the error() method.
    \return Returns <code>true</code> if the runtime was successfully initialized; <code>false</code> otherwise.
    */
    bool isValid() const;

    /**
    \brief Returns the last error that occured in this runtime.
    \return A StorageError that indicates the cause of the error if isValid() returns <code>false</code>.
    If isValid() returns <code>true</code>, the returned StorageError has type StorageError::NoError.
    */
    StorageError error() const;

    /**
    \brief Returns the DBus connection used by this runtime.
    \return The <a href="http://doc.qt.io/qt-5/qdbusconnection.html">QDBusConnection</a> used by this runtime
    to access providers.
    */
    QDBusConnection connection() const;

    /**
    \brief Shuts down the runtime.

    This method is provided to allow you to check whether the runtime was successfully finalized and log
    any errors. If errors were encountered during shutdown, isValid() returns <code>false</code>
    and you can call error() to get the details of the problem.

    Calling shutdown() more than once is safe and does nothing.

    The destructor implicitly calls shutdown().

    Any calls to other instances obtained via this runtime after
    calling shutdown() fail with StorageError::RuntimeDestroyed.
    \return Details of any error encountered during shutdown.
    */
    StorageError shutdown();

    /**
    \brief Retrieves the list of available accounts.
    \return An AccountsJob that, once complete, provides access to the available accounts.
    \note You <i>must</i> deallocate the returned job by calling <code>delete</code>.
    \see Account
    */
    Q_INVOKABLE unity::storage::qt::AccountsJob* accounts() const;

    /**
    \brief Creates a test account.

    This method is intended for testing. You can use the returned Account to talk to a provider that
    listens on DBus with the provided bus name and object path.
    (If you want to test a client application without the overhead of accessing a cloud provider or creating
    a mock provider, we suggest to use the \link local-provider local provider\endlink.)
    \param bus_name The DBus name of the provider for the account.
    \param object_path The DBus object path of the provider for the account.
    \param id The numeric ID of the account.
    \param service_id The account service ID, such as "storage-provider-nextcloud".
    \param name The name of the account, such as "user@domain.com".
    \return The test account.
    */
    Account make_test_account(QString const& bus_name,
                              QString const& object_path,
                              quint32 id = 999,
                              QString const& service_id = "",
                              QString const& name = "") const;

private:
    std::shared_ptr<internal::RuntimeImpl> p_;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
