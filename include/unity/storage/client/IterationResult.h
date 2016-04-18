#pragma once

//#include "Item.h"

namespace unity
{

namespace storage
{

namespace client
{

class IterationResult
{
public:
    ~IterationResult();
    IterationResult(IterationResult const&) = delete;
    IterationResult& operator=(IterationResult const&) = delete;
    IterationResult(IterationResult&&) = delete;
    IterationResult& operator=(IterationResult&&) = delete;

    Item::UPtr get_item() const;

private:
    IterationResult();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
