#pragma once

#include "Directory.h"

namespace unity
{

namespace storage
{

namespace client
{

class ParentResult
{
public:
    ~ParentResult();
    ParentResult(ParentResult const&) = delete;
    ParentResult& operator=(ParentResult const&) = delete;
    ParentResult(ParentResult&&) = delete;
    ParentResult& operator=(ParentResult&&) = delete;

    Directory::UPtr get_directory() const;

private:
    ParentResult();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
