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
    LAST_ENTRY__
};

enum class ConflictPolicy
{
    error_if_conflict,
    overwrite,
};

}  // namespace storage
}  // namespace unity
