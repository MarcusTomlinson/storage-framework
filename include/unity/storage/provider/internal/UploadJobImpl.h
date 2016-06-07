#pragma once

#include <unity/storage/provider/visibility.h>

#include <QObject>

#include <string>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

class UploadJobImpl : public QObject
{
    Q_OBJECT
public:
    explicit UploadJobImpl(std::string const& upload_id);
    virtual ~UploadJobImpl();

    std::string const& upload_id() const;
    int read_socket() const;
    int take_write_socket();

    std::string const& sender_bus_name() const;
    void set_sender_bus_name(std::string const& bus_name);

protected:
    std::string const upload_id_;
    int read_socket_ = -1;
    int write_socket_ = -1;
    std::string sender_bus_name_;
};

}
}
}
}
