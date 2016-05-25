#pragma once

#include <unity/storage/visibility.h>

#include <QException>

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
class UNITY_STORAGE_EXPORT StorageException : public QException
{
public:
    StorageException();
    ~StorageException();

    virtual StorageException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates errors in the communication with the storage provider service.
*/
class UNITY_STORAGE_EXPORT LocalCommsException : public StorageException
{
public:
    LocalCommsException();
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
    RemoteCommsException();
    ~RemoteCommsException();

    virtual RemoteCommsException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that the caller invoked an operation on an item that was destroyed.
*/
class UNITY_STORAGE_EXPORT DestroyedException : public StorageException
{
public:
    DestroyedException();
    ~DestroyedException();

    virtual DestroyedException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that the caller destroyed the runtime.
*/
class UNITY_STORAGE_EXPORT RuntimeDestroyedException : public StorageException
{
public:
    RuntimeDestroyedException();
    ~RuntimeDestroyedException();

    virtual RuntimeDestroyedException* clone() const override;
    virtual void raise() const override;
};

/**
\brief Indicates that an item does not exist.
*/
class UNITY_STORAGE_EXPORT NotExistException : public StorageException
{
public:
    NotExistException();
    ~NotExistException();

    virtual NotExistException* clone() const override;
    virtual void raise() const override;
};

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
