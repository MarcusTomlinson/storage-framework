#pragma once

#include "Item.h"

namespace unity
{

namespace storage
{

namespace client
{

class CreateResult;
class IterationResult;
class ParentResult;

/**
\brief Class that represents a directory.

A directory is an unordered set of files and/or directories. The names (as returned by Item::name())
of the members of a directory are unique.
*/

class Directory : public Item
{
public:
    ~Directory();
    Directory(Directory const&) = delete;
    Directory& operator=(Directory const&) = delete;
    Directory(Directory&&);
    Directory& operator=(Directory&&);

    typedef std::unique_ptr<Directory> UPtr;

    /**
    \brief Returns the contents of a directory.
    \param iteration_callback As results become available, the runtime
    calls iteration_callback once for each item. The iteration functor
    can call IterationResult::get_item() to retrieve the item. A `nullptr`
    return value from `get_item()` indicates end-of-iteration.
    */
    void list(std::function<void(IterationResult const&)> iteration_callback);

    /**
    \brief Returns the list of parent directories of this directory.
    \param parents_callback As parent directories become available, the runtime
    calls parents_callback once for each parent directory. The parents functor
    can call ParentResult::get_directory() to retrieve the Directory. A `nullptr`
    return value from `get_directory()` indicates end-of-iteration.
    */
    void parents(std::function<void(ParentResult const&)> parents_callback);

    /**
    \brief Creates a new directory with the current directory as the parent.
    \param name The name of the new directory. Note that the actual name may be changed
    by the provider; call Item::name() once the directory is created to get its actual name.
    \param completion_callback Once the directory is created, the runtime calls completion_callback.
    The completion functor can call CreateResult::get_item() to access the new directory.
    \note Error information is returned as exceptions that are thrown from `get_item`.
    */
    void create_dir(std::string const& name, std::function<void(CreateResult const&)> completion_callback);

    /**
    \brief Creates a new empty file with the current directory as the parent.
    \param name The name of the new file. Note that the actual name may be changed
    by the provider; call Item::name() once the file is created to get its actual name.
    \param completion_callback Once the file is created, the runtime calls completion_callback.
    The completion functor can call CreateResult::get_item() to access the new file.
    \note Error information is returned as exceptions that are thrown from `get_item`.
    */
    void create_file(std::string const& name, std::function<void(CreateResult const&)> completion_callback);

private:
    Directory();
};

}  // namespace client

}  // namespace storage

}  // namespace unity
