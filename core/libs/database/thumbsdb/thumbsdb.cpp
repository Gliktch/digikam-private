/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Thumbnails database interface.
 *
 * SPDX-FileCopyrightText:      2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbsdb.h"

// Qt includes

#include <QMap>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN ThumbsDb::Private
{

public:

    Private() = default;

public:

    ThumbsDbBackend* db = nullptr;
};

ThumbsDb::ThumbsDb(ThumbsDbBackend* const backend)
    : d(new Private)
{
    d->db = backend;
}

ThumbsDb::~ThumbsDb()
{
    delete d;
}

bool ThumbsDb::setSetting(const QString& keyword, const QString& value )
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    parameters.insert(QLatin1String(":value"), value);
    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("ReplaceThumbnailSetting")),
                                                                       parameters);

    return (queryStateResult == BdEngineBackend::NoErrors);
}

QString ThumbsDb::getSetting(const QString& keyword)
{
    QVariantList            values;
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);

    // TODO Should really check return status here

    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("SelectThumbnailSetting")),
                                                                       parameters, &values);

    qCDebug(DIGIKAM_THUMBSDB_LOG) << "ThumbDB SelectThumbnailSetting val ret = "
                                  << (BdEngineBackend::QueryStateEnum)queryStateResult;

    if (!values.isEmpty())
    {
        return values.first().toString();
    }

    return QString();
}

QString ThumbsDb::getLegacySetting(const QString& keyword)
{
    QVariantList            values;
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);

    // TODO Should really check return status here

    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("SelectThumbnailLegacySetting")),
                                                                       parameters, &values);

    qCDebug(DIGIKAM_THUMBSDB_LOG) << "ThumbDB SelectThumbnailLegacySetting val ret = "
                                  << (BdEngineBackend::QueryStateEnum)queryStateResult;

    if (!values.isEmpty())
    {
        return values.first().toString();
    }

    return QString();
}

ThumbsDbInfo ThumbsDb::fillThumbnailInfo(const QVariantList& values)
{
    if (values.isEmpty())
    {
        return ThumbsDbInfo();
    }

    ThumbsDbInfo info;

    info.id               = values.at(0).toInt();
    info.type             = (DatabaseThumbnail::Type)values.at(1).toInt();
    info.modificationDate = asDateTimeUTC(values.at(2).toDateTime());
    info.orientationHint  = values.at(3).toInt();
    info.data             = values.at(4).toByteArray();

    return info;
}

ThumbsDbInfo ThumbsDb::findByHash(const QString& uniqueHash, qlonglong fileSize)
{
    QVariantList values;

    d->db->execSql(QLatin1String("SELECT id, type, modificationDate, orientationHint, data "
                                 "FROM Thumbnails "
                                 " INNER JOIN UniqueHashes ON id = thumbId "
                                 "  WHERE uniqueHash=? AND fileSize=?;"),
                   uniqueHash, fileSize, &values);

    return fillThumbnailInfo(values);
}

ThumbsDbInfo ThumbsDb::findByFilePath(const QString& path)
{
    QVariantList values;

    d->db->execSql(QLatin1String("SELECT id, type, modificationDate, orientationHint, data "
                                 "FROM Thumbnails "
                                 " INNER JOIN FilePaths ON id = thumbId "
                                 "  WHERE path=?;"),
                   path, &values);

    return fillThumbnailInfo(values);
}

ThumbsDbInfo ThumbsDb::findByFilePath(const QString& path, const QString& uniqueHash)
{
    ThumbsDbInfo info = findByFilePath(path);

    if (uniqueHash.isNull())
    {
        return info;
    }

    if (info.data.isNull())
    {
        return info;
    }

    // double check that thumbnail is not referenced by a different hash

    QVariantList values;

    d->db->execSql(QLatin1String("SELECT uniqueHash FROM UniqueHashes WHERE thumbId=?;"),
                   info.id, &values);

    if (values.isEmpty())
    {
        return info;
    }

    for (const QVariant& hash : std::as_const(values))
    {
        if (hash == uniqueHash)
        {   // cppcheck-suppress useStlAlgorithm
            return info;
        }
    }

    return ThumbsDbInfo();
}

ThumbsDbInfo ThumbsDb::findByCustomIdentifier(const QString& id)
{
    QVariantList values;

    d->db->execSql(QLatin1String("SELECT id, type, modificationDate, orientationHint, data "
                                 "FROM Thumbnails "
                                 " INNER JOIN CustomIdentifiers ON id = thumbId "
                                 "  WHERE identifier=?;"),
                   id, &values);

    return fillThumbnailInfo(values);
}

QList<int> ThumbsDb::findAll()
{
    QVariantList values;

    d->db->execSql(QLatin1String("SELECT id FROM Thumbnails;"),
                   &values);

    QList<int> thumbIds;

    for (const QVariant& object : std::as_const(values))
    {
        thumbIds << object.toInt();
    }

    return thumbIds;
}

QHash<QString, int> ThumbsDb::getFilePathsWithThumbnail()
{
    QSqlQuery query = d->db->prepareQuery(QString::fromLatin1("SELECT path, thumbId "
                                                              "FROM FilePaths "
                                                              " INNER JOIN Thumbnails ON thumbId = id "
                                                              "  WHERE type BETWEEN %1 AND %2;")
                                          .arg(DatabaseThumbnail::PGF)
                                          .arg(DatabaseThumbnail::PNG));

    if (!d->db->exec(query))
    {
        return QHash<QString, int>();
    }

    QHash <QString, int> filePaths;

    while (query.next())
    {
        filePaths[query.value(0).toString()] = query.value(1).toInt();
    }

    return filePaths;
}

BdEngineBackend::QueryState ThumbsDb::insertUniqueHash(const QString& uniqueHash, qlonglong fileSize, int thumbId)
{
    return d->db->execSql(QLatin1String("REPLACE INTO UniqueHashes (uniqueHash, fileSize, thumbId) VALUES (?,?,?);"),
                          uniqueHash, fileSize, thumbId);
}

BdEngineBackend::QueryState ThumbsDb::insertFilePath(const QString& path, int thumbId)
{
    return d->db->execSql(QLatin1String("REPLACE INTO FilePaths (path, thumbId) VALUES (?,?);"),
                          path, thumbId);
}

BdEngineBackend::QueryState ThumbsDb::insertCustomIdentifier(const QString& path, int thumbId)
{
    return d->db->execSql(QLatin1String("REPLACE INTO CustomIdentifiers (identifier, thumbId) VALUES (?,?);"),
                          path, thumbId);
}

BdEngineBackend::QueryState ThumbsDb::remove(int thumbId)
{
    return d->db->execSql(QLatin1String("DELETE FROM Thumbnails WHERE id=?;"), thumbId);
}

BdEngineBackend::QueryState ThumbsDb::removeByUniqueHash(const QString& uniqueHash, qlonglong fileSize)
{
    // UniqueHashes + FilePaths entries are removed by trigger

    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":uniqueHash"), uniqueHash);
    parameters.insert(QLatin1String(":filesize"),   fileSize);

    return d->db->execDBAction(d->db->getDBAction(QLatin1String("Delete_Thumbnail_ByUniqueHashId")),
                               parameters);
}

BdEngineBackend::QueryState ThumbsDb::removeByFilePath(const QString& path)
{
    // UniqueHashes + FilePaths entries are removed by trigger

    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":path"), path);

    return d->db->execDBAction(d->db->getDBAction(QLatin1String("Delete_Thumbnail_ByPath")),
                               parameters);
}

BdEngineBackend::QueryState ThumbsDb::removeByCustomIdentifier(const QString& id)
{
    // UniqueHashes + FilePaths entries are removed by trigger

    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":identifier"), id);

    return d->db->execDBAction(d->db->getDBAction(QLatin1String("Delete_Thumbnail_ByCustomIdentifier")),
                               parameters);
}

BdEngineBackend::QueryState ThumbsDb::insertThumbnail(const ThumbsDbInfo& info, QVariant* const lastInsertId)
{
    QVariant id;
    BdEngineBackend::QueryState lastQueryState;
    lastQueryState = d->db->execSql(QLatin1String("INSERT INTO Thumbnails (type, modificationDate, orientationHint, data) VALUES (?, ?, ?, ?);"),
                                    info.type, d->db->asDBDateTime(info.modificationDate), info.orientationHint, info.data, nullptr, &id);

    if (BdEngineBackend::NoErrors == lastQueryState)
    {
        *lastInsertId = id.toInt();
    }
    else
    {
        *lastInsertId = -1;
    }

    return lastQueryState;
}

BdEngineBackend::QueryState ThumbsDb::renameByFilePath(const QString& oldPath, const QString& newPath)
{
    QVariantList values;

    d->db->execSql(QLatin1String("SELECT id FROM Thumbnails "
                                 " INNER JOIN FilePaths ON id = thumbId "
                                 "  WHERE path=?;"),
                   newPath, &values);

    if (!values.isEmpty())
    {
        removeByFilePath(newPath);
    }

    return d->db->execSql(QLatin1String("UPDATE FilePaths SET path=? WHERE path=?;"),
                          newPath, oldPath);
}

BdEngineBackend::QueryState ThumbsDb::replaceThumbnail(const ThumbsDbInfo& info)
{
    return d->db->execSql(QLatin1String("REPLACE INTO Thumbnails (id, type, modificationDate, orientationHint, data) VALUES(?, ?, ?, ?, ?);"),
                          QList<QVariant>() << info.id << info.type
                                            << d->db->asDBDateTime(info.modificationDate)
                                            << info.orientationHint << info.data);
}

BdEngineBackend::QueryState ThumbsDb::updateModificationDate(int thumbId, const QDateTime& modificationDate)
{
    return d->db->execSql(QLatin1String("UPDATE Thumbnails SET modificationDate=? WHERE id=?;"),
                          d->db->asDBDateTime(modificationDate), thumbId);
}

void ThumbsDb::replaceUniqueHash(const QString& oldUniqueHash, int oldFileSize,
                                 const QString& newUniqueHash, int newFileSize)
{
    d->db->execSql(QLatin1String("UPDATE UniqueHashes SET uniqueHash=?, fileSize=? WHERE uniqueHash=? AND fileSize=?;"),
                   newUniqueHash, newFileSize, oldUniqueHash, oldFileSize);
}

bool ThumbsDb::integrityCheck()
{
    QVariantList values;

    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("checkThumbnailsDbIntegrity")),
                        &values);

    switch (d->db->databaseType())
    {
        case BdEngineBackend::DbType::SQLite:
        {
            // For SQLite the integrity check returns a single row with one string column "ok" on success and multiple rows on error.

            bool result = (
                           (values.size() == 1) &&
                           (values.first().toString().toLower().compare(QLatin1String("ok")) == 0)
                          );

            if (!result && !values.isEmpty())
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Failed integrity check for SQLite thumbnail database:"
                                                << values.first().toString();
            }

            return result;
        }

        case BdEngineBackend::DbType::MySQL:
        {
            // For MySQL, for every checked table, the table name, operation (check), message type (status) and the message text (ok on success)
            // are returned. So we check if there are four elements and if yes, whether the fourth element is "ok".
/*
            qCDebug(DIGIKAM_DATABASE_LOG) << "MySQL check returned " << values.size() << " rows";
*/
            if ((values.size() % 4) != 0)
            {
                return false;
            }

            for (QList<QVariant>::iterator it = values.begin() ; it != values.end() ; )
            {
                QString tableName   = (*it).toString();
                ++it;
                QString operation   = (*it).toString();
                ++it;
                QString messageType = (*it).toString();
                ++it;
                QString messageText = (*it).toString();
                ++it;

                Q_UNUSED(operation);

                if (!messageText.isEmpty())
                {
                    if ((messageType.toLower().compare(QLatin1String("note")) == 0))
                    {
                        qCWarning(DIGIKAM_DATABASE_LOG) << "Failed integrity check for table "
                                                        << tableName << ". Reason:" << messageText;
                    }

                    if (
                        (messageType.toLower().compare(QLatin1String("status")) == 0) &&
                        (messageText.toLower().compare(QLatin1String("ok"))     != 0)
                       )
                    {
                        return false;
                    }
                }
                else
                {
/*
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Passed integrity check for table " << tableName;
*/
                }
            }

            // No error conditions. Db passed the integrity check.

            return true;
        }

        default:
        {
            return false;
        }
    }
}

void ThumbsDb::vacuum()
{
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("vacuumThumbnailsDB")));
}

} // namespace Digikam
