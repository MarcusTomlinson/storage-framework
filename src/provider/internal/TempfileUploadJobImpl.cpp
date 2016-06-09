#include <unity/storage/provider/internal/TempfileUploadJobImpl.h>

#include <cassert>
#include <stdexcept>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

TempfileUploadJobImpl::TempfileUploadJobImpl(std::string const& upload_id)
    : UploadJobImpl(upload_id)
{
}

TempfileUploadJobImpl::~TempfileUploadJobImpl() = default;

void TempfileUploadJobImpl::complete_init()
{
    tmpfile_ = new QTemporaryFile(this);
    reader_ = new QLocalSocket(this);

    assert(tmpfile_->open());

    reader_->setSocketDescriptor(
        read_socket_, QLocalSocket::ConnectedState, QIODevice::ReadOnly);
    read_socket_ = -1;
    connect(reader_, &QIODevice::readyRead,
            this, &TempfileUploadJobImpl::on_ready_read);
    connect(reader_, &QIODevice::readChannelFinished,
            this, &TempfileUploadJobImpl::on_read_channel_finished);
}

std::string TempfileUploadJobImpl::file_name() const
{
    if (!tmpfile_)
    {
        return "";
    }
    return tmpfile_->fileName().toStdString();
}

void TempfileUploadJobImpl::on_ready_read()
{
    char buffer[4096];
    while (reader_->bytesAvailable() > 0)
    {
        qint64 n_read = reader_->read(buffer, sizeof(buffer));
        if (n_read > 0)
        {
            qint64 n_written = tmpfile_->write(buffer, n_read);
            assert(n_written == n_read);
        }
    }
}

void TempfileUploadJobImpl::on_read_channel_finished()
{
    // drain the socket and close the tempfile.
    on_ready_read();
    tmpfile_->close();
}

}
}
}
}
