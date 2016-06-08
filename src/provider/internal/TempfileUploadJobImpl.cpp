#include <unity/storage/provider/internal/TempfileUploadJobImpl.h>

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
    // Relying on Qt parent-child memory management, and to make sure
    // they have the same thread affinity.
    reader_ = new QLocalSocket(this);
    tmpfile_ = new QTemporaryFile(this);

    if (!tmpfile_->open())
    {
        throw runtime_error("Could not open tempfile: " + tmpfile_->errorString().toStdString());
    }

    reader_->setSocketDescriptor(
        read_socket_, QLocalSocket::ConnectedState, QIODevice::ReadOnly);
    read_socket_ = -1;
    connect(reader_, &QIODevice::readyRead,
            this, &TempfileUploadJobImpl::on_ready_read);
    connect(reader_, &QIODevice::readChannelFinished,
            this, &TempfileUploadJobImpl::on_read_channel_finished);
}

TempfileUploadJobImpl::~TempfileUploadJobImpl() = default;

std::string TempfileUploadJobImpl::file_name() const
{
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
            tmpfile_->write(buffer, n_read);
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
