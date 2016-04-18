#pragma once

#include "Directory.h"

namespace unity
{

namespace storage
{

namespace client
{

// How do we do metadata? As an object with a fixed set of strongly-typed accessors, one for each attribute?
// As a boost::variant? As JSON?
// Different providers have dramatically different metadata. Do we try to define a common set?

typedef int MetadataMap;

class MetadataResult
{
public:
    ~MetadataResult();
    MetadataResult(MetadataResult const&) = delete;
    MetadataResult& operator=(MetadataResult const&) = delete;
    MetadataResult(MetadataResult&&) = delete;
    MetadataResult& operator=(MetadataResult&&) = delete;

    MetadataMap get_metadata() const;

private:
    MetadataResult();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
