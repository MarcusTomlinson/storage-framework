#include <unity/storage/qt/client/Exceptions.h>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{

StorageException::StorageException() = default;

StorageException::~StorageException() = default;

StorageException* StorageException::clone() const
{
    return new StorageException(*this);
}

void StorageException::raise() const
{
    throw *this;
}

LocalCommsException::LocalCommsException() = default;

LocalCommsException::~LocalCommsException() = default;

LocalCommsException* LocalCommsException::clone() const
{
    return new LocalCommsException(*this);
}

void LocalCommsException::raise() const
{
    throw *this;
}

RemoteCommsException::RemoteCommsException() = default;

RemoteCommsException::~RemoteCommsException() = default;

RemoteCommsException* RemoteCommsException::clone() const
{
    return new RemoteCommsException(*this);
}

void RemoteCommsException::raise() const
{
    throw *this;
}

DestroyedException::DestroyedException() = default;

DestroyedException::~DestroyedException() = default;

DestroyedException* DestroyedException::clone() const
{
    return new DestroyedException(*this);
}

void DestroyedException::raise() const
{
    throw *this;
}

RuntimeDestroyedException::RuntimeDestroyedException() = default;

RuntimeDestroyedException::~RuntimeDestroyedException() = default;

RuntimeDestroyedException* RuntimeDestroyedException::clone() const
{
    return new RuntimeDestroyedException(*this);
}

void RuntimeDestroyedException::raise() const
{
    throw *this;
}

NotExistException::NotExistException() = default;

NotExistException::~NotExistException() = default;

NotExistException* NotExistException::clone() const
{
    return new NotExistException(*this);
}

void NotExistException::raise() const
{
    throw *this;
}

ConflictException::ConflictException() = default;

ConflictException::~ConflictException() = default;

ConflictException* ConflictException::clone() const
{
    return new ConflictException(*this);
}

void ConflictException::raise() const
{
    throw *this;
}

CancelledException::CancelledException() = default;

CancelledException::~CancelledException() = default;

CancelledException* CancelledException::clone() const
{
    return new CancelledException(*this);
}

void CancelledException::raise() const
{
    throw *this;
}

}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
