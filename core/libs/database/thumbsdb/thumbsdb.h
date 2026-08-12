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

#pragma once

// Qt includes

#include <QDateTime>
#include <QString>
#include <QHash>
#include <QList>

// Local includes

#include "thumbsdbbackend.h"
#include "thumbsdbaccess.h"
#include "digikam_export.h"

namespace Digikam
{

namespace DatabaseThumbnail
{

enum Type
{
    UndefinedType = 0,
    NoThumbnail,
    PGF,
    JPEG,              // Warning : no alpha channel support. Cannot be used as well.
    JPEG2000,
    PNG
    //FreeDesktopHash
};

} // namespace DatabaseThumbnail

class DIGIKAM_EXPORT ThumbsDbInfo
{

public:

    ThumbsDbInfo() = default;

    int                     id                  = -1;
    DatabaseThumbnail::Type type                = DatabaseThumbnail::UndefinedType;
    QDateTime               modificationDate;
    int                     orientationHint     = 0;
    QByteArray              data;
};

// ------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbsDb
{

public:

    bool setSetting(const QString& keyword, const QString& value);
    QString getSetting(const QString& keyword);
    QString getLegacySetting(const QString& keyword);

    ThumbsDbInfo findByHash(const QString& uniqueHash, qlonglong fileSize);
    ThumbsDbInfo findByFilePath(const QString& path);
    ThumbsDbInfo findByCustomIdentifier(const QString& id);

    /**
     * This is findByFilePath with extra security: Pass the uniqueHash which you have.
     * If an entry is found by file path, and the entry is referenced by any uniqueHash,
     * which is different from the given hash, a null info is returned.
     * If uniqueHash is null, equivalent to the simple findByFilePath.
     */
    ThumbsDbInfo findByFilePath(const QString& path, const QString& uniqueHash);

    /**
     * Returns the thumbnail ids of all thumbnails in the database.
     */
    QList<int> findAll();

    BdEngineBackend::QueryState insertUniqueHash(const QString& uniqueHash, qlonglong fileSize, int thumbId);
    BdEngineBackend::QueryState insertFilePath(const QString& path, int thumbId);
    BdEngineBackend::QueryState insertCustomIdentifier(const QString& id, int thumbId);

    BdEngineBackend::QueryState remove(int thumbId);

    /**
     * Removes thumbnail data associated to the given uniqueHash/fileSize
     */
    BdEngineBackend::QueryState removeByUniqueHash(const QString& uniqueHash, qlonglong fileSize);

    /**
     * Removes thumbnail data associated to the given file path
     */
    BdEngineBackend::QueryState removeByFilePath(const QString& path);
    BdEngineBackend::QueryState removeByCustomIdentifier(const QString& id);

    BdEngineBackend::QueryState renameByFilePath(const QString& oldPath, const QString& newPath);

    BdEngineBackend::QueryState insertThumbnail(const ThumbsDbInfo& info, QVariant* const lastInsertId = nullptr);
    BdEngineBackend::QueryState replaceThumbnail(const ThumbsDbInfo& info);

    QHash<QString, int> getFilePathsWithThumbnail();

    void replaceUniqueHash(const QString& oldUniqueHash, int oldFileSize, const QString& newUniqueHash, int newFileSize);
    BdEngineBackend::QueryState updateModificationDate(int thumbId, const QDateTime& modificationDate);

    // ----------- Database shrinking methods ----------

    /**
     * Returns true if the integrity of the database is preserved.
     */
    bool integrityCheck();

    /**
     * Shrinks the database.
     */
    void vacuum();

private:

    explicit ThumbsDb(ThumbsDbBackend* const backend);
    ~ThumbsDb();

    ThumbsDbInfo fillThumbnailInfo(const QVariantList& values);

private:

    /// @note disabled
    ThumbsDb(const ThumbsDb&)            = delete;
    ThumbsDb& operator=(const ThumbsDb&) = delete;

    class Private;
    Private* const d = nullptr;

    friend class ThumbsDbAccess;
};

} // namespace Digikam
