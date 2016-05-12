#pragma once

#include <unity/storage/provider/visibility.h>

#include <boost/thread/future.hpp>

#include <string>
#include <map>
#include <vector>

namespace unity
{
namespace storage
{
namespace provider
{

enum class ItemType
{
    file,
    folder,
    root,
};

struct STORAGE_PROVIDER_EXPORT Item
{
    std::string item_id;
    std::string parent_id;
    std::string title;
    std::string etag;
    ItemType type;
    // Should be map<string,variant>
    std::map<std::string,std::string> metadata;
};

typedef std::vector<Item> ItemList;

class STORAGE_PROVIDER_EXPORT ProviderBase
{
public:
    ProviderBase();
    virtual ~ProviderBase();

    virtual boost::future<ItemList> roots() = 0;
    virtual boost::future<std::tuple<ItemList,std::string>> list(
        std::string const& item_id, std::string const& page_token) = 0;
    virtual boost::future<ItemList> lookup(
        std::string const& parent_id, std::string const& name) = 0;
    virtual boost::future<Item> metadata(std::string const& item_id) = 0;
};

}
}
}
