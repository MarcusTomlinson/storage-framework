#pragma once

#include "Item.h"

namespace unity
{

namespace storage
{

namespace client
{

class DownloadResult
{
public:
    ~DownloadResult();
    DownloadResult(DownloadResult const&) = delete;
    DownloadResult& operator=(DownloadResult const&) = delete;
    DownloadResult(DownloadResult&&) = delete;
    DownloadResult& operator=(DownloadResult&&) = delete;

    /**
    Checks for errors once a download has completed.

    If the download has completed, this method returns normally. Otherwise, it throws an exception
    that provides more details about the error.
    \throws TODO: list of exceptions
    */
    void check_error() const;

private:
    DownloadResult();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
