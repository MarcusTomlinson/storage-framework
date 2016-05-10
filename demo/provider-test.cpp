
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/Server.h>

#include <stdexcept>

using namespace unity::storage::provider;

class MyProvider : public ProviderBase
{
public:
    MyProvider();

    boost::future<std::vector<Item>> roots() override;
    boost::future<std::tuple<std::vector<Item>,std::string>> list(
        std::string const& item_id, std::string const& page_token) override;
    boost::future<std::vector<Item>> lookup(
        std::string const& parent_id, std::string const& name) override;
    boost::future<Item> metadata(std::string const& item_id) override;
};

MyProvider::MyProvider()
{
}

boost::future<std::vector<Item>> MyProvider::roots()
{
    std::vector<Item> roots = {
        {"root_id", "", "Root", "etag", ItemType::root, {}},
    };
    return boost::make_ready_future<std::vector<Item>>(roots);
}

boost::future<std::tuple<std::vector<Item>,std::string>> MyProvider::list(
    std::string const& item_id, std::string const& page_token)
{
    if (item_id != "root_id")
    {
        return boost::make_exceptional_future<std::tuple<std::vector<Item>,std::string>>(std::runtime_error("unknown folder"));
    }
    if (page_token != "")
    {
        return boost::make_exceptional_future<std::tuple<std::vector<Item>,std::string>>(std::runtime_error("unknown page token"));
    }
    std::vector<Item> children = {
        {"child_id", "root_id", "Child", "etag", ItemType::file, {}}
    };
    boost::promise<std::tuple<std::vector<Item>,std::string>> p;
    p.set_value(std::make_tuple(children, std::string()));
    return p.get_future();
}

boost::future<std::vector<Item>> MyProvider::lookup(
    std::string const& parent_id, std::string const& name)
{
    if (parent_id != "root_id" || name != "Child")
    {
        return boost::make_exceptional_future<std::vector<Item>>(std::runtime_error("file not found"));
    }
    std::vector<Item> children = {
        {"child_id", "root_id", "Child", "etag", ItemType::file, {}}
    };
    return boost::make_ready_future<std::vector<Item>>(children);
}

boost::future<Item> MyProvider::metadata(std::string const& item_id)
{
    if (item_id == "root_id")
    {
        Item metadata{"root_id", "", "Root", "etag", ItemType::root, {}};
        return boost::make_ready_future<Item>(metadata);
    }
    else if (item_id == "child_id")
    {
        Item metadata{"child_id", "root_id", "Child", "etag", ItemType::file, {}};
        return boost::make_ready_future<Item>(metadata);
    }
    return boost::make_exceptional_future<Item>(std::runtime_error("no such file"));
}

int main(int argc, char **argv)
{
    Server<MyProvider> server;
    server.init(argc, argv);
    server.run();
}
