#pragma once

#include <unity/storage/visibility.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtConcurrent/QtConcurrent>
#pragma GCC diagnostic pop

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

/**
\brief Base exception class for all exceptions returned by the API.
*/
class UNITY_STORAGE_EXPORT StorageException : public QException //QtConcurrent::Exception
{
public:
    StorageException(QString const& error_message);
    ~StorageException();

    virtual StorageException* clone() const = 0;
    virtual void raise() const = 0;

private:
    QString error_message_;
};

/**
\brief Indicates errors in the communication with the storage provider service.
*/
class UNITY_STORAGE_EXPORT LocalCommsException : public StorageException
{
public:
    LocalCommsException(QString const& error_message);
    ~LocalCommsException();

    virtual LocalCommsException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates errors in the communication between the storage provider and the cloud service.
*/
class UNITY_STORAGE_EXPORT RemoteCommsException : public StorageException
{
public:
    RemoteCommsException(QString const& error_message);
    ~RemoteCommsException();

    virtual RemoteCommsException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that the caller invoked an operation on a file or folder that was deleted.
*/
class UNITY_STORAGE_EXPORT DeletedException : public StorageException
{
public:
    DeletedException(QString const& error_message, QString const& identity_, QString const& name_);
    ~DeletedException();

    virtual DeletedException* clone() const override;
    virtual void raise() const override;

    QString native_identity() const;
    QString name() const;

private:
    QString identity_;
    QString name_;
};

/**
\brief Indicates that the caller destroyed the runtime.
*/
class UNITY_STORAGE_EXPORT RuntimeDestroyedException : public StorageException
{
public:
    RuntimeDestroyedException(QString const& method);
    ~RuntimeDestroyedException();

    virtual RuntimeDestroyedException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that an item does not exist or could not be found.
*/
class UNITY_STORAGE_EXPORT NotExistsException : public StorageException
{
public:
    NotExistsException(QString const& error_message, QString const& key);
    ~NotExistsException();

    virtual NotExistsException* clone() const override;
    virtual void raise() const override;

    QString key() const;

private:
    QString key_;
};

/**
\brief Indicates that an item cannot be created because it exists already.
*/
class UNITY_STORAGE_EXPORT ExistsException : public StorageException
{
public:
    ExistsException(QString const& error_message, QString const& identity, QString const& name);
    ~ExistsException();

    virtual ExistsException* clone() const override;
    virtual void raise() const override;

    QString native_identity() const;
    QString name() const;

private:
    QString identity_;
    QString name_;
};

/**
\brief Indicates that an upload detected a version mismatch.
*/
class UNITY_STORAGE_EXPORT ConflictException : public StorageException
{
public:
    ConflictException(QString const& error_message);
    ~ConflictException();

    virtual ConflictException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that an upload or download was cancelled before it could complete.
*/
class UNITY_STORAGE_EXPORT CancelledException : public StorageException
{
public:
    CancelledException(QString const& error_message);
    ~CancelledException();

    virtual CancelledException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates incorrect use of the API, such as calling methods in the wrong order.
*/
class UNITY_STORAGE_EXPORT LogicException : public StorageException
{
public:
    LogicException(QString const& error_message);
    ~LogicException();

    virtual LogicException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates an invalid parameter, such as a negative value when a positive one was
expected, or a string that does not parse correctly or is empty when it should be non-empty.
*/
class UNITY_STORAGE_EXPORT InvalidArgumentException : public StorageException
{
public:
    InvalidArgumentException(QString const& error_message);
    ~InvalidArgumentException();

    virtual InvalidArgumentException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates a system error, such as failure to to create a file or folder,
or any other (usually non-recoverable) kind of error that should not arise during normal operation.
*/
class UNITY_STORAGE_EXPORT ResourceException : public StorageException
{
public:
    ResourceException(QString const& error_message);
    ~ResourceException();

    virtual ResourceException* clone() const override;
    virtual void raise() const override;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
