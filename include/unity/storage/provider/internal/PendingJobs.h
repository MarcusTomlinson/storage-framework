#pragma once

#include <QObject>

#include <map>
#include <memory>
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

class UploadJob;

namespace internal
{


class PendingJobs : public QObject
{
    Q_OBJECT

public:
    PendingJobs(QObject *parent=nullptr);
    virtual ~PendingJobs();

    void add_upload(std::unique_ptr<UploadJob> &&job);

    std::shared_ptr<UploadJob> get_upload(std::string const& upload_id);
    void remove_upload(std::string const& upload_id);

private:
    std::map<std::string,std::shared_ptr<UploadJob>> uploads_;
};

}
}
}
}
