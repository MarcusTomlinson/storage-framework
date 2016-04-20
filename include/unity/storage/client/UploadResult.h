#pragma once

#include <unity/storage/client/Item.h>

namespace unity
{

namespace storage
{

namespace client
{

class UploadResult
{
public:
    ~UploadResult();
    UploadResult(UploadResult const&) = delete;
    UploadResult& operator=(UploadResult const&) = delete;
    UploadResult(UploadResult&&) = delete;
    UploadResult& operator=(UploadResult&&) = delete;

    /**
    Checks for errors once an upload has completed.

    If the download has completed, this method returns normally. Otherwise, it throws an exception
    that provides more details about the error.
    \throws TODO: list of exceptions
    */
    void check_error() const;

private:
    UploadResult();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
