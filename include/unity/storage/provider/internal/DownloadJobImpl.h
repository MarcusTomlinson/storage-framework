#pragma once

#include <QObject>

#include <boost/thread/future.hpp>

#include <mutex>
#include <stdexcept>
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{
class DownloadJob;

namespace internal
{

class DownloadJobImpl : public QObject
{
    Q_OBJECT
public:
    explicit DownloadJobImpl(std::string const& download_id);
    virtual ~DownloadJobImpl();

    std::string const& download_id() const;
    int write_socket() const;
    int take_read_socket();

    std::string const& sender_bus_name() const;
    void set_sender_bus_name(std::string const& bus_name);

    void report_complete();
    void report_error(std::exception_ptr p);
    boost::future<void> finish(DownloadJob& job);
    boost::future<void> cancel(DownloadJob& job);

public Q_SLOTS:
    virtual void complete_init();

protected:
    std::string const download_id_;
    int read_socket_ = -1;
    int write_socket_ = -1;
    std::string sender_bus_name_;

    std::mutex completion_lock_;
    bool completed_ = false;
    boost::promise<void> completion_promise_;

    Q_DISABLE_COPY(DownloadJobImpl)
};

}
}
}
}
