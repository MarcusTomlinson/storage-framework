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

#include <QObject>

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class VoidJobImpl;

}  // namespace internal

class StorageError;

/**
\brief Asynchronous job to monitor progress of an operation that returns no result.
*/

class Q_DECL_EXPORT VoidJob final : public QObject
{
    Q_OBJECT

    /**
    \see \link isValid() const isValid()\endlink
    */
    Q_PROPERTY(bool isValid READ isValid NOTIFY statusChanged FINAL)

    /**
    \see \link status() const status()\endlink
    */
    Q_PROPERTY(unity::storage::qt::VoidJob::Status status READ status NOTIFY statusChanged FINAL)

    /**
    \see \link error() const error()\endlink
    */
    Q_PROPERTY(unity::storage::qt::StorageError error READ error NOTIFY statusChanged FINAL)

public:
    /**
    \brief Destroys the job.

    It is safe to destroy a job while it is still executing.
    */
    virtual ~VoidJob();

    /**
    \brief Indicates the status of the job.
    */
    enum Status {
        Loading,   /*!< The job is still executing. */
        Finished,  /*!< The job finished succesfully. */
        Error      /*!< The job finished with an error. */
    };
    Q_ENUMS(Status)

    /**
    \brief Returns whether this job was successfully created.
    \return If the job status is \link Error\endlink, the return value is <code>false</code>;
    <code>true</code> otherwise.
    */
    bool isValid() const;

    /**
    \brief Returns the current job status.
    \return The job status.
    */
    Status status() const;

    /**
    \brief Returns the last error that occured in this job.
    \return A StorageError that indicates the cause of the error if isValid() returns <code>false</code>.
    If isValid() returns <code>true</code>, the returned StorageError has type StorageError::NoError.
    */
    StorageError error() const;

Q_SIGNALS:
    /** @name Signals
    */
    //{@
    /**
    \brief This signal is emitted whenever this job transitions to the \link Finished\endlink or \link Error\endlink state.
    \param status The status of the job.
    */
    void statusChanged(unity::storage::qt::VoidJob::Status status) const;
    //@}

private:
    ///@cond
    VoidJob(std::unique_ptr<internal::VoidJobImpl> p);

    std::unique_ptr<internal::VoidJobImpl> const p_;

    friend class internal::VoidJobImpl;
    ///@endcond
};

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::VoidJob::Status)
