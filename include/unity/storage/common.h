#pragma once

namespace unity
{
namespace storage
{

enum class ItemType
{
    file,
    folder,
    root,
};

enum class TransferState
{
    ok,
    cancelled
};

}  // namespace storage
}  // namespace unity
