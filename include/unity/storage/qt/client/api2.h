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

#include <QIODevice>

class QDBusConnection;

// TODO: Split this header into several headers once we know what we want and adjust forward declarations.

namespace unity
{
namespace storage
{
namespace qt
{

class AccountsJob;
class CancelJob;
class Downloader;
class DownloadJob;
class Item;
class ItemJob;
class ItemListJob;
class Runtime;
class Uploader;
class UploadJob;
class VoidJob;

class StorageError final
{
    Q_GADGET
    Q_PROPERTY(ErrorType type READ type)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString message READ message)
    Q_PROPERTY(QString errorString READ errorString)

    Q_PROPERTY(QString itemId READ itemId)
    Q_PROPERTY(QString itemName READ itemName)
    Q_PROPERTY(int errorCode READ errorCode)

public:
    StorageError();
    StorageError(StorageError const&);
    StorageError(StorageError&&);
    ~StorageError();
    StorageError& operator=(StorageError const&);
    StorageError& operator=(StorageError&&);

    enum class ErrorType
    {
        NoError, Invalid, LocalCommsError, RemoteCommsError, Deleted, RuntimeDestroyed, NotExists,
        Exists, Conflict, PermissionDenied, Cancelled, LogicError, InvalidArgument, ResourceError
    };
    Q_ENUM(ErrorType)

    ErrorType type() const;
    QString name() const;
    QString message() const;
    QString errorString() const;

    QString itemId() const;
    QString itemName() const;
    int errorCode() const;
};

class Account final
{
    Q_GADGET
    Q_PROPERTY(QString READ owner)
    Q_PROPERTY(QString READ ownerId)
    Q_PROPERTY(QString READ description)

public:
    Account();
    Account(Account const&);
    Account(Account&&);
    ~Account();
    Account& operator=(Account const&);
    Account& operator=(Account&&);

    QString owner() const;
    QString ownerId() const;
    QString description() const;

    Q_INVOKABLE ItemListJob* roots() const;

    bool operator==(Account const&) const;
    bool operator!=(Account const&) const;
    bool operator<(Account const&) const;
    bool operator<=(Account const&) const;
    bool operator>(Account const&) const;
    bool operator>=(Account const&) const;
};

bool operator==(Account const&, Account const&);
bool operator!=(Account const&, Account const&);
bool operator<(Account const&, Account const&);
bool operator<=(Account const&, Account const&);
bool operator>(Account const&, Account const&);
bool operator>=(Account const&, Account const&);

class Runtime : public QObject
{
    Q_OBJECT
public:
    virtual ~Runtime();

    static Runtime create();
    static Runtime create(QDBusConnection const& bus);

    Q_INVOKABLE AccountsJob* accounts() const;

private:
    Runtime();
};

class Downloader : public QIODevice
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(StorageError error READ error NOTIFY error)
    Q_PROPERTY(Item item READ item NOTIFY finished)

public:
    enum class Status { Loading, Cancelled, Finished, Error };
    Q_ENUM(Status)

    bool isValid();
    Status status() const;
    StorageError error() const;
    Item item() const;

    Q_INVOKABLE void finishDownload();
    Q_INVOKABLE void cancel();
    
Q_SIGNALS:
    void statusChanged(Status status) const;
    void error(StorageError const& e) const;
    void finished(Item const& item) const;
    void cancelled() const;
};

class Uploader : public QIODevice
{
    Q_OBJECT
    Q_PROPERTY(bool isValid READ isValid)
    Q_PROPERTY(Status status READ status)
    Q_PROPERTY(error item READ error)
    Q_PROPERTY(ConflictPolicy policy READ policy)
    Q_PROPERTY(qint64 sizeInBytes READ sizeInBytes)
    Q_PROPERTY(Item item READ item)

public:
    enum class ConflictPolicy { ErrorIfConflict, Overwrite };
    Q_ENUM(ConflictPolicy)

    enum class Status { Loading, Cancelled, Finished, Error };
    Q_ENUM(Status)

    bool isValid() const;
    Status status() const;
    StorageError error() const;
    ConflictPolicy policy() const;
    qint64 sizeInBytes() const;
    Item item() const;

    Q_INVOKABLE void finishUpload();
    Q_INVOKABLE void cancel();

Q_SIGNALS:
    void statusChanged(Status status) const;
    void error(StorageError const& e) const;
    void finished(Item const& item) const;
    void cancelled() const;
};

class Item final
{
    Q_GADGET
    Q_PROPERTY(QString itemId READ itemId CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString parentId READ parentId CONSTANT)
    Q_PROPERTY(Account account READ account CONSTANT)
    Q_PROPERTY(Item root READ root CONSTANT)
    Q_PROPERTY(QString eTag READ eTag CONSTANT)
    Q_PROPERTY(ItemType type READ type CONSTANT)
    Q_PROPERTY(QVariantMap metadata READ metadata CONSTANT)
    Q_PROPERTY(QDateTime lastModifiedTime READ lastModifiedTime CONSTANT)
    Q_PROPERTY(QVector<QString> parentIds READ parentIds CONSTANT)

public:
    Item();
    Item(Item const&);
    Item(Item&&);
    ~Item();
    Item& operator=(Item const&);
    Item& operator=(Item&&);

    bool isValid() const;

    enum class Type { File, Folder, Root };
    Q_ENUM(Type)

    QString itemId() const;
    QString name() const;
    QString parentId() const;
    Account account() const;
    Item root() const;
    QString eTag() const;
    Type type() const;
    QVariantMap metadata() const;
    QDateTime lastModifiedTime() const;
    QVector<QString> parentIds() const;

    Q_INVOKABLE ItemListJob* parents() const;
    Q_INVOKABLE ItemJob* copy(Item const& new_parent, QString const& new_name) const;
    Q_INVOKABLE ItemJob* move(Item const& new_parent, QString const& new_name) const;
    Q_INVOKABLE VoidJob* delete_item() const;

    Q_INVOKABLE Uploader* createUploader(Uploader::ConflictPolicy policy, qint64 sizeInBytes) const;
    Q_INVOKABLE Downloader* createDownloader() const;

    Q_INVOKABLE ItemListJob* list() const;
    Q_INVOKABLE ItemListJob* lookup(QString const& name) const;
    Q_INVOKABLE ItemJob* create_folder(QString const& name) const;
    Q_INVOKABLE UploadJob* create_file(QString const& name) const;

    Q_INVOKABLE ItemJob* get(QString itemId) const;
    Q_INVOKABLE qint64 freeSpaceBytes() const;
    Q_INVOKABLE qint64 usedSpaceBytes() const;

    bool operator==(Item const&) const;
    bool operator!=(Item const&) const;
    bool operator<(Item const&) const;
    bool operator<=(Item const&) const;
    bool operator>(Item const&) const;
    bool operator>=(Item const&) const;
};

bool operator==(Item const&, Item const&);
bool operator!=(Item const&, Item const&);
bool operator<(Item const&, Item const&);
bool operator<=(Item const&, Item const&);
bool operator>(Item const&, Item const&);
bool operator>=(Item const&, Item const&);

class AccountsJob : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Status READ status NOTIFY statusChanged)
    Q_PROPERTY(StorageError READ Error NOTIFY error)
    Q_PROPERTY(QList<Account> READ accounts)

public:
    AccountsJob(QObject*);
    virtual ~AccountsJob();

    enum class Status { Loading, Finished, Error };
    Q_ENUM(Status)

    Status status() const;
    StorageError error() const;
    QList<Account> accounts() const;
    
Q_SIGNALS:
    void statusChanged(Status status) const;
    void error(StorageError const& e) const;
    void finished(QList<Account> const& accounts) const;
};

class ItemJob : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool READ isValid)
    Q_PROPERTY(Status READ status NOTIFY statusChanged)
    Q_PROPERTY(StorageError READ error NOTIFY error)
    Q_PROPERTY(Item const& READ item NOTIFY finished)

public:
    ItemJob(QObject* parent = nullptr);
    virtual ~ItemJob();

    enum class Status { Loading, Finished, Error };
    Q_ENUM(Status)

    bool isValid() const;
    Status status() const;
    StorageError error() const;
    Item item() const;

Q_SIGNALS:
    void statusChanged(Status status) const;
    void error(StorageError const& e) const;
    void finished(Item const& item) const;
};

class ItemListJob : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Status READ status NOTIFY statusChanged)
    Q_PROPERTY(StorageError READ Error NOTIFY error)

public:
    ItemListJob(QObject* parent = nullptr);
    virtual ~ItemListJob();

    enum class Status { Loading, Finished, Error };
    Q_ENUM(Status)

    bool isValid() const;
    Status status() const;
    StorageError error() const;

Q_SIGNALS:
    void statusChanged(Status status) const;
    void error(StorageError const& e) const;
    void resultsReady(QList<Item> const& results) const;
    void finished() const;
};

class VoidJob : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Status READ status NOTIFY statusChanged)
    Q_PROPERTY(StorageError READ Error NOTIFY error)

public:
    VoidJob(QObject* parent = nullptr);
    virtual ~VoidJob();

    enum class Status { Loading, Finished, Error };
    Q_ENUM(Status)

Q_SIGNALS:
    void statusChanged(Status status) const;
    void error(StorageError const& e) const;
    void finished() const;
};

}  // namespace qt
}  // namespace storage
}  // namespace unity
