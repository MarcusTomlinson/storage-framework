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

#include <unity/storage/visibility.h>

#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

// Note: Adding new exception types also requires updating the
//       marshaling and unmarshaling code for exceptions in the client
//       and server APIs.  In particular:
//         - src/provider/internal/Handler.cpp
//         - src/provider/internal/utils.cpp
//         - src/qt/internal/unmarshal_error.cpp

/**
\brief Abstract base exception class for all server-side exceptions.
*/

class UNITY_STORAGE_EXPORT StorageException : public std::exception
{
public:
    /**
    \brief Constructs a StorageException.
    \param exception_type The concrete type name of the exception, such as "ConflictException".
    \param error_message The error message text for the exception.
    */
    StorageException(std::string const& exception_type, std::string const& error_message);

protected:
    ~StorageException();

public:
    /**
    \brief Returns an error string.
    \return The error message prefixed by the exception type, such as "ConflictException: ETag mismatch".
    */
    virtual char const* what() const noexcept override;

    /**
    \brief Returns the exception type name.
    \return The value of the <code>exception_type</code> parameter that was passed to the constructor.
    */
    std::string type() const;

    /**
    \brief Returns the except error message.
    \return The value of the <code>error_message</code> parameter that was passed to the constructor.
    */
    std::string error_message() const;

private:
    std::string what_string_;
    std::string type_;
    std::string error_message_;
};

/**
\brief Indicates errors in the communication between the storage provider and the cloud service.

Use this exception for unexpected communication errors, such as the remote server being unreachable
or returning garbled data.
*/

class UNITY_STORAGE_EXPORT RemoteCommsException : public StorageException
{
public:
    /**
    \brief Construct a RemoteCommsException.
    \param error_message The error message for the exception.
    */
    RemoteCommsException(std::string const& error_message);
    ~RemoteCommsException();
};

/**
\brief Indicates that an item does not exist.

Use this exception only if the remote cloud service has <i>authoritatively</i> indicated non-existence
(such as with an HTTP 404 response). Do not use this exception for non-authoritative errors, such as
timeouts or similar.
*/

class UNITY_STORAGE_EXPORT NotExistsException : public StorageException
{
public:
    /**
    \brief Construct a RemoteCommsException.
    \param error_message The error message for the exception.
    \param key The name or identity of the non-existent item.
    */
    NotExistsException(std::string const& error_message, std::string const& key);
    ~NotExistsException();

    /**
    \brief Return the key.
    \return The value of the <code>key</code> parameter that was passed to the constructor.
    */
    std::string key() const;

private:
    std::string key_;
};

/**
\brief Indicates that an item already exists.

Use this exception only if the remote cloud service has <i>authoritatively</i> indicated that an operation
cannot be carried out because an item exists already. Do not use this exception for non-authoritative errors,
such as timeouts or similar.
*/

class UNITY_STORAGE_EXPORT ExistsException : public StorageException
{
public:
    /**
    \brief Construct an ExistsException.
    \param error_message The error message for the exception.
    \param identity The identity of the item.
    \param name The name of the item.
    */
    ExistsException(std::string const& error_message, std::string const& identity, std::string const& name);
    ~ExistsException();

    /**
    \brief Return the identity.
    \return The value of the <code>identity</code> parameter that was passed to the constructor.
    */
    std::string native_identity() const;

    /**
    \brief Return the name of the item.
    \return The value of the <code>name</code> parameter that was passed to the constructor.
    */
    std::string name() const;

private:
    std::string identity_;
    std::string name_;
};

/**
\brief Indicates that an upload or download detected a ETag mismatch.
*/

class UNITY_STORAGE_EXPORT ConflictException : public StorageException
{
public:
    /**
    \brief Construct a ConflictException.
    \param error_message The error message for the exception.
    */
    ConflictException(std::string const& error_message);
    ~ConflictException();
};

/**
\brief Indicates that an operation failed because the credentials are invalid or have expired.

A provider implementation must throw this exception if it cannot reach
its provider because the credentials are invalid. Do <i>not</i> throw this
exception if the credentials are valid, but an operation failed due to
insufficient permission for an item (such as an attempt to write to a
read-only file).

Typically, this exception will cause the runtime to retry the operation after refreshing
the credentials; the exception may be returned to the client on repeated failures.

\see PermissionException
*/

class UNITY_STORAGE_EXPORT UnauthorizedException : public StorageException
{
public:
    /**
    \brief Construct an UnauthorizedException.
    \param error_message The error message for the exception.
    */
    UnauthorizedException(std::string const& error_message);
    ~UnauthorizedException();
};

/**
\brief Indicates that an operation failed because of a permission problem.

A provider implementation must throw this exception if it can
authenticate with its provider, but the provider denied the operation
due to insufficient permission for an item (such as an attempt to
write to a read-only file). Do <i>not</i> throw this exception for failure to
authenticate with the provider.

\see UnauthorizedException
*/

class UNITY_STORAGE_EXPORT PermissionException : public StorageException
{
public:
    /**
    \brief Construct a PermissionException.
    \param error_message The error message for the exception.
    */
    PermissionException(std::string const& error_message);
    ~PermissionException();
};

/**
\brief Indicates that an update failed because the provider ran out of space or exceeded
the maximum number of files or folders.
*/

class UNITY_STORAGE_EXPORT QuotaException : public StorageException
{
public:
    /**
    \brief Construct a QuotaException.
    \param error_message The error message for the exception.
    */
    QuotaException(std::string const& error_message);
    ~QuotaException();
};

/**
\brief Indicates that an upload or download was cancelled before it could complete.
\note Due to the way the provider API is structured, you will not ever need to throw this exception. It is provided for
completeness and to allow for provider implementations that do not use the storage framework API.
*/

class UNITY_STORAGE_EXPORT CancelledException : public StorageException
{
public:
    /**
    \brief Construct a CancelledException.
    \param error_message The error message for the exception.
    */
    CancelledException(std::string const& error_message);
    ~CancelledException();
};

/**
\brief Indicates incorrect use of the API.

Use this exception for errors that the client could avoid, such as calling methods in the wrong order or attempting
to list the contents of file (as opposed to a folder).
\note Do <i>not</i> throw this exception to indicate arguments that are malformed or out of range.
\see InvalidArgumentException
*/
class UNITY_STORAGE_EXPORT LogicException : public StorageException
{
public:
    /**
    \brief Construct a LogicException.
    \param error_message The error message for the exception.
    */
    LogicException(std::string const& error_message);
    ~LogicException();
};

/**
\brief Indicates an invalid parameter.

Use this exception for errors such as a negative parameter value when a positive one was
expected, or a string that does not parse correctly or is empty when it should be non-empty.
\note Do <i>not</i> throw this exception to indicate incorrect use of the API.
\see LogicException
*/

class UNITY_STORAGE_EXPORT InvalidArgumentException : public StorageException
{
public:
    /**
    \brief Construct an InvalidArgumentException.
    \param error_message The error message for the exception.
    */
    InvalidArgumentException(std::string const& error_message);
    ~InvalidArgumentException();
};

/**
\brief Indicates a system error.

This is a generic "catch-all" exception that you can throw to indicate unexpected errors that do not fit into
any of the other categories. For example, ResourceException is appropriate to indicate out of
file descriptors, failure to locate a configuration file, or any other unexpected system error. You should
provide as much contextual information about such errors as possible. In particular, unexpected errors
typically need to be diagnosed from log files. This means that you should provide, at least, the full error
message you received from the cloud service (or the operating system), together with all other relevant
details, such as the name of the file, the URL of a failed request, any HTTP error information, and so on.
*/

class UNITY_STORAGE_EXPORT ResourceException : public StorageException
{
public:
    /**
    \brief Construct an InvalidArgumentException.
    \param error_message The error message for the exception.
    \param error_code The system (or library) error code for the exception.
    In case of system call failures, <code>error_code</code> should be the value of <code>errno</code>.
    If the error code represents something else, such as a <code>QFileDevice::FileError</code>,
    also indicate in the error message what that error code is; otherwise, it can be
    impossible to diagnose a problem, especially when looking at log files after the fact.
    */
    ResourceException(std::string const& error_message, int error_code);
    ~ResourceException();

    /**
    \brief Return the error code.
    \return The value of the <code>error_code</code> parameter that was passed to the constructor.
    */
    int error_code() const noexcept;

private:
    int error_code_;
};

/**
\brief Indicates that a provider threw an exception not derived from StorageException.

The runtime translates all exception that are thrown by a provider and that do not derive from StorageException
to UnknownException.

Do not throw this exception explicitly; is is a "catch-all" exception that the runtime uses to
indicate that something completely unexpected happened. (If a client receives this exception from your
provider, chances are that you have forgotten to handle an error condition somewhere.)
*/

class UNITY_STORAGE_EXPORT UnknownException : public StorageException
{
public:
    /**
    \brief Construct an UnknownArgumentException.
    \param error_message The error message for the exception. This is the string returned by <code>what()</code>
    for <code>std::exception</code> or "Unknown exception" for exceptions that do not derive
    from <code>std::exception</code>.
    */
    UnknownException(std::string const& error_message);
    ~UnknownException();
};

}  // namespace provider
}  // namespace storage
}  // namespace unity
