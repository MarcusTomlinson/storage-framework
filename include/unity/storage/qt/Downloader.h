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
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <unity/storage/qt/Item.h>
#include <unity/storage/qt/StorageError.h>

#include <QLocalSocket>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class DownloaderImpl;

}  // namespace internal

/**
\brief Class to download the contents of a file.
*/

class Q_DECL_EXPORT Downloader final : public QIODevice
{
    Q_OBJECT

    /**
    \see \link isValid() const isValid()\endlink
    */
    Q_PROPERTY(bool isValid READ isValid NOTIFY statusChanged FINAL)

    /**
    \see \link status() const status()\endlink
    */
    Q_PROPERTY(unity::storage::qt::Downloader::Status status READ status NOTIFY statusChanged FINAL)

    /**
    \see \link error() const error()\endlink
    */
    Q_PROPERTY(unity::storage::qt::StorageError error READ error NOTIFY statusChanged FINAL)

    /**
    \see \link item() const item()\endlink
    */
    Q_PROPERTY(unity::storage::qt::Item item READ item NOTIFY statusChanged FINAL)

public:
    /**
    \brief Indicates the status of the job.
    */
    enum Status {
        Loading,   /*!< The job is still executing. */
        Ready,     /*!< The job is ready for reading data. */
        Cancelled, /*!< The job was cancelled. */
        Finished,  /*!< The job finished succesfully. */
        Error      /*!< The job finished with an error. */
    };
    Q_ENUMS(Status)

    Downloader();
    virtual ~Downloader();

    bool isValid() const;
    Status status() const;
    StorageError error() const;
    Item item() const;

    Q_INVOKABLE void cancel();

    // From QLocalSocket interface.
    Q_INVOKABLE void close();
    Q_INVOKABLE qint64 bytesAvailable() const override;
    Q_INVOKABLE qint64 bytesToWrite() const override;
    Q_INVOKABLE bool canReadLine() const override;
    Q_INVOKABLE bool isSequential() const override;
    Q_INVOKABLE bool waitForBytesWritten(int msecs = 30000) override;
    Q_INVOKABLE bool waitForReadyRead(int msecs = 30000) override;

Q_SIGNALS:
    /** @name Signals
    */
    //{@
    /**
    \brief This signal is emitted whenever this job transitions to a new state.
    \param status The status of the job.
    */
    void statusChanged(unity::storage::qt::Downloader::Status status) const;
    //@}

private:
    ///@cond
    Downloader(std::unique_ptr<internal::DownloaderImpl> p);

    qint64 readData(char* data, qint64 c);
    qint64 writeData(char const* data, qint64 c);

    std::unique_ptr<internal::DownloaderImpl> p_;

    friend class internal::DownloaderImpl;
    ///@endcond
};

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::Downloader::Status)
