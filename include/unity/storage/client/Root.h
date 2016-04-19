#pragma once

#include "Directory.h"

namespace unity
{

namespace storage
{

namespace client
{

class Result;  // Same as CreateResult?

/**
\brief Class that represents a root directory.
*/

class Root : public Directory
{
public:
    ~Root();
    Root(Root const&) = delete;
    Root& operator=(Root const&) = delete;
    Root(Root&&);
    Root& operator=(Root&&);

    typedef std::unique_ptr<Root> UPtr;

    int64_t free_space_bytes() const;  // Needs to be async, make part of metadata?
    int64_t used_space_bytes() const;  // Needs to be async, make part of metadata?

    void get(std::string native_identity, std::function<void(Result)> result_callback) const;

private:
    Root();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
