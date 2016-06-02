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

protected:
    std::string const upload_id_;
    int read_socket_ = -1;
    int write_socket_ = -1;
};

}
}
}
}
