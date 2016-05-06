#define BOOST_THREAD_VERSION 4
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

struct ItemMetadata
{
    std::string item_id;
    std::string parent_id;
    std::string title;
    std::string etag;
    ItemType type;
    // Should be map<string,variant>
    std::map<std::string,std::string> metadata;
};

class ProviderBase {
public:
    ProviderBase();
    virtual ~ProviderBase();

    virtual boost::future<std::vector<ItemMetadata>> roots() = 0;
};

}
}
}
