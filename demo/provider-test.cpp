
#include <unity/storage/provider/ProviderBase.h>
#include <unity/storage/provider/Server.h>

#include <cstdio>
#include <stdexcept>

using namespace unity::storage::provider;

class MyProvider : public ProviderBase
{
public:
    MyProvider();

    boost::future<ItemList> roots(Context const& ctx) override;
    boost::future<std::tuple<ItemList,std::string>> list(
        std::string const& item_id, std::string const& page_token,
        Context const& ctx) override;
    boost::future<ItemList> lookup(
        std::string const& parent_id, std::string const& name,
        Context const& ctx) override;
    boost::future<Item> metadata(
        std::string const& item_id, Context const& ctx) override;
};

MyProvider::MyProvider()
{
}

boost::future<ItemList> MyProvider::roots(Context const& ctx)
{
    printf("roots() called by %s (%d)\n", ctx.security_label.c_str(), ctx.pid);
    ItemList roots = {
        {"root_id", "", "Root", "etag", ItemType::root, {}},
    };
    return boost::make_ready_future<ItemList>(roots);
}

boost::future<std::tuple<ItemList,std::string>> MyProvider::list(
    std::string const& item_id, std::string const& page_token,
    Context const& ctx)
{
    printf("list('%s', '%s') called by %s (%d)\n", item_id.c_str(), page_token.c_str(), ctx.security_label.c_str(), ctx.pid);
    if (item_id != "root_id")
    {
        return boost::make_exceptional_future<std::tuple<ItemList,std::string>>(std::runtime_error("unknown folder"));
    }
    if (page_token != "")
    {
        return boost::make_exceptional_future<std::tuple<ItemList,std::string>>(std::runtime_error("unknown page token"));
    }
    ItemList children = {
        {"child_id", "root_id", "Child", "etag", ItemType::file, {}}
    };
    boost::promise<std::tuple<ItemList,std::string>> p;
    p.set_value(std::make_tuple(children, std::string()));
    return p.get_future();
}

boost::future<ItemList> MyProvider::lookup(
    std::string const& parent_id, std::string const& name, Context const& ctx)
{
    printf("lookup('%s', '%s') called by %s (%d)\n", parent_id.c_str(), name.c_str(), ctx.security_label.c_str(), ctx.pid);
    if (parent_id != "root_id" || name != "Child")
    {
        return boost::make_exceptional_future<ItemList>(std::runtime_error("file not found"));
    }
    ItemList children = {
        {"child_id", "root_id", "Child", "etag", ItemType::file, {}}
    };
    return boost::make_ready_future<ItemList>(children);
}

boost::future<Item> MyProvider::metadata(std::string const& item_id,
                                         Context const& ctx)
{
    printf("metadata('%s') called by %s (%d)\n", item_id.c_str(), ctx.security_label.c_str(), ctx.pid);
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
    Server<MyProvider> server("com.canonical.StorageFramework.Provider.ProviderTest", "google-drive-scope");
    server.init(argc, argv);
    server.run();
}
