#pragma once

#include "Item.h"

namespace unity
{

namespace storage
{

namespace client
{

class CreateResult
{
public:
    ~CreateResult();
    CreateResult(CreateResult const&) = delete;
    CreateResult& operator=(CreateResult const&) = delete;
    CreateResult(CreateResult&&) = delete;
    CreateResult& operator=(CreateResult&&) = delete;

    Item::UPtr get_item() const;

private:
    CreateResult();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
