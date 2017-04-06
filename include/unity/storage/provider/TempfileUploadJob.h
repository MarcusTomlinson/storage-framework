/*
 * Copyright (C) 2016 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#pragma once

#include <unity/storage/provider/UploadJob.h>
#include <unity/storage/visibility.h>

#include <string>
#include <memory>

namespace unity
{
namespace storage
{
namespace provider
{

namespace internal
{
class TempfileUploadJobImpl;
}

/**
\brief Helper class to store the contents of an upload in a temporary file.

For uploads, a provider implementation must decide whether to upload data to the cloud provider
immediately (as soon as the data arrives from the client) or whether to buffer the data locally first
and upload it to the cloud provider once the client successfully finalizes the upload. Uploading
immediately has the down side that it is more likely to fail. This is an issue particularly
for mobile devices, where applications may be suspended for extended periods, and where connectivity
is often lost without warning. Uploads of large files (such as media files) are unlikely
to ever complete in this case: if the user swipes away from the client application, no more data arrives
at the provider until the application resumes, by which time the connection to the cloud provider has
most likely timed out.

Especially for files that are more than a few kilobytes in size, it is usually necessary to write the data
to a local file first and to write it to the cloud provider only once all of data has been sent by the client.
(The provider service does not get suspended and will not exit while an upload is in progress.)

TempfileUploadJob is a helper class that implements an uploader that writes the data to a temporary file.
The file is unlinked once the TempfileUploadJob is destroyed. You implementation must provide finish() and
cancel().

*/

class UNITY_STORAGE_EXPORT TempfileUploadJob : public UploadJob
{
public:
    /**
    \brief Construct a TempfileUploadJob.
    \param upload_id An identifier for this particular upload. You can use any non-empty string,
    as long as it is unique among all uploads that are in progress within this provider.
    The runtime uses the <code>upload_id</code> to distinguish different uploads that may be
    in progress concurrently.
    */
    TempfileUploadJob(std::string const& upload_id);
    virtual ~TempfileUploadJob();

    /**
    \brief Returns the name of the file.
    \return The full path name of the temporary file.
    */
    std::string file_name() const;

    /**
    \brief Reads any unread data from the upload socket and writes it to the temporary file.

    You must call drain() from your UploadJob::finish() method. Its implementation reads any remaining
    data from the upload socket and writes it to the temporary file. If an error occurs,
    drain() throws an exception.
    \throws LogicException The client did not close its end of the socket.
    */
    void drain();

private:
    TempfileUploadJob(internal::TempfileUploadJobImpl *p) UNITY_STORAGE_HIDDEN;
};

}
}
}
