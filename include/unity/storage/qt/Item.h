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

#include <unity/storage/common.h>
#include <unity/storage/qt/MetadataKeys.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDateTime>
#include <QVariantMap>
#pragma GCC diagnostic pop

#include <memory>

namespace unity
{
namespace storage
{
namespace qt
{
namespace internal
{

class ItemImpl;
class DownloaderImpl;
class UploaderImpl;

}  // namespace internal

class Account;
class Downloader;
class IntJob;
class ItemJob;
class ItemListJob;
class Uploader;
class VoidJob;

class Q_DECL_EXPORT Item final
{
    Q_GADGET
    Q_PROPERTY(QString itemId READ itemId FINAL)
    Q_PROPERTY(QString name READ name FINAL)
    Q_PROPERTY(unity::storage::qt::Account account READ account FINAL)
    Q_PROPERTY(QString etag READ etag FINAL)
    Q_PROPERTY(unity::storage::qt::Item::Type type READ type FINAL)
    Q_PROPERTY(QVariantMap metadata READ metadata FINAL)
    Q_PROPERTY(QDateTime lastModifiedTime READ lastModifiedTime FINAL)
    Q_PROPERTY(QList<QString> parentIds READ parentIds FINAL)

public:
    Item();
    Item(Item const&);
    Item(Item&&);
    ~Item();
    Item& operator=(Item const&);
    Item& operator=(Item&&);

    enum Type
    {
        File = unsigned(unity::storage::ItemType::file),
        Folder = unsigned(unity::storage::ItemType::folder),
        Root = unsigned(unity::storage::ItemType::root)
    };
    Q_ENUMS(Type)

    enum ConflictPolicy
    {
        ErrorIfConflict = unsigned(unity::storage::ConflictPolicy::error_if_conflict),
        Overwrite = unsigned(unity::storage::ConflictPolicy::overwrite)
    };
    Q_ENUMS(ConflictPolicy)

    bool isValid() const;
    QString itemId() const;
    QString name() const;
    Account account() const;
    QString etag() const;
    Type type() const;
    QVariantMap metadata() const;
    QDateTime lastModifiedTime() const;
    QList<QString> parentIds() const;

    Q_INVOKABLE ItemListJob* parents(MetadataKeys const& keys = MetadataKeys()) const;
    Q_INVOKABLE ItemJob* copy(Item const& newParent,
                              QString const& newName,
                              MetadataKeys const& keys = MetadataKeys()) const;
    Q_INVOKABLE ItemJob* move(Item const& newParent,
                              QString const& newName,
                              MetadataKeys const& keys = MetadataKeys()) const;
    Q_INVOKABLE VoidJob* deleteItem() const;

    Q_INVOKABLE Uploader* createUploader(ConflictPolicy policy,
                                         qint64 sizeInBytes,
                                         MetadataKeys const& keys = MetadataKeys()) const;
    Q_INVOKABLE Downloader* createDownloader() const;

    Q_INVOKABLE ItemListJob* list(MetadataKeys const& keys = MetadataKeys()) const;
    Q_INVOKABLE ItemListJob* lookup(QString const& name, MetadataKeys const& = MetadataKeys()) const;
    Q_INVOKABLE ItemJob* createFolder(QString const& name, MetadataKeys const& = MetadataKeys()) const;
    Q_INVOKABLE Uploader* createFile(QString const& name,
                                     ConflictPolicy policy,
                                     qint64 sizeInBytes,
                                     QString const& contentType,
                                     MetadataKeys const& keys = MetadataKeys()) const;

    Q_INVOKABLE IntJob* freeSpaceBytes() const;
    Q_INVOKABLE IntJob* usedSpaceBytes() const;

    bool operator==(Item const&) const;
    bool operator!=(Item const&) const;
    bool operator<(Item const&) const;
    bool operator<=(Item const&) const;
    bool operator>(Item const&) const;
    bool operator>=(Item const&) const;

    size_t hash() const;

private:
    Item(std::shared_ptr<internal::ItemImpl> const&);

    std::shared_ptr<internal::ItemImpl> p_;

    friend class internal::ItemImpl;
    friend class internal::DownloaderImpl;
    friend class internal::UploaderImpl;
};

// Note: qHash(Item) does *not* return the same hash value is std::hash<Item> because
//       std:hash() returns size_t (typically 64 bits), but qHash() returns uint (typically 32 bits).
uint Q_DECL_EXPORT qHash(unity::storage::qt::Item const& i);

}  // namespace qt
}  // namespace storage
}  // namespace unity

Q_DECLARE_METATYPE(unity::storage::qt::Item)
Q_DECLARE_METATYPE(QList<unity::storage::qt::Item>)
Q_DECLARE_METATYPE(unity::storage::qt::Item::Type)
Q_DECLARE_METATYPE(unity::storage::qt::Item::ConflictPolicy)

namespace std
{

template<> struct Q_DECL_EXPORT hash<unity::storage::qt::Item>
{
    std::size_t operator()(unity::storage::qt::Item const& i) const
    {
        return i.hash();
    }
};

}  // namespace std
