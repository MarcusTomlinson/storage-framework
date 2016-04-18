#pragma once

namespace unity
{

namespace storage
{

namespace client
{

class Item;

class DestroyResult
{
public:
    ~DestroyResult();
    DestroyResult(DestroyResult const&) = delete;
    DestroyResult& operator=(DestroyResult const&) = delete;
    DestroyResult(DestroyResult&&) = delete;
    DestroyResult& operator=(DestroyResult&&) = delete;

    Item const& get_item() const;  // Dubious, what if Item is no longer there?

private:
    DestroyResult();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
