#pragma once

#include <unity/storage/provider/internal/UploadJobImpl.h>

#include <QLocalSocket>
#include <QTemporaryFile>

#include <string>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

class TempfileUploadJobImpl : public UploadJobImpl
{
    Q_OBJECT
public:
    explicit TempfileUploadJobImpl(std::string const& upload_id);
    virtual ~TempfileUploadJobImpl();

    std::string file_name() const;

private Q_SLOTS:
    void on_ready_read();
    void on_read_channel_finished();

private:
    QLocalSocket *reader_;
    QTemporaryFile *tmpfile_;

    Q_DISABLE_COPY(TempfileUploadJobImpl)
};

}
}
}
}
