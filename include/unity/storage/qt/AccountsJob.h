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

#include <unity/storage/qt/Account.h>
#include <unity/storage/qt/StorageError.h>

#include <QMetaType>
#include <QObject>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class AccountsJobImpl;

}  // namespace internal

class Account;
class StorageError;

/**
\brief Asynchronous job to retrieve a list of accounts.
*/

class Q_DECL_EXPORT AccountsJob final : public QObject
{
    Q_OBJECT

    /**
    \see \link isValid() const isValid()\endlink
    */
    Q_PROPERTY(bool isValid READ isValid NOTIFY statusChanged FINAL)

    /**
    \see \link status() const status()\endlink
    */
    Q_PROPERTY(unity::storage::qt::AccountsJob::Status status READ status NOTIFY statusChanged FINAL)

    /**
    \see \link error() const error()\endlink
    */
    Q_PROPERTY(unity::storage::qt::StorageError error READ error NOTIFY statusChanged FINAL)

    /**
    \see \link accounts() const accounts()\endlink
    */
    Q_PROPERTY(QVariantList accounts READ accountsAsVariantList NOTIFY statusChanged FINAL)

public:
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
    \brief Destroys the job.

    It is safe to destroy a job while it is still executing.
    */
    virtual ~AccountsJob();

    /**
    \brief Returns whether a job was successfully created.
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

    /**
    \brief Returns the list of accounts.
    \return The list of accounts or an empty list if the status is not \link Finished\endlink.
    */
    QList<Account> accounts() const;

Q_SIGNALS:
    /**
    \brief This signal is emitted whenever a job transitions to the \link Finished\endlink or \link Error\endlink state.
    \param status The status of the job.
    */
    void statusChanged(unity::storage::qt::AccountsJob::Status status) const;

private:
    ///@cond
    AccountsJob(std::unique_ptr<internal::AccountsJobImpl> accounts_job_impl);

    QVariantList accountsAsVariantList() const;

    std::unique_ptr<internal::AccountsJobImpl> const p_;

    friend class internal::AccountsJobImpl;
    ///@endcond
};

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::AccountsJob::Status)
