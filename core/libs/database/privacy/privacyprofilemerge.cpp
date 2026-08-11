/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyprofilemerge.h"

// Qt includes

#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QUuid>

namespace Digikam
{

namespace
{

QString connectionNameFor(const char* purpose)
{
    return QString::fromLatin1(purpose) +
           QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool nextId(QSqlDatabase& database, const QString& table, int* const id)
{
    QSqlQuery query(database);

    if (!query.exec(QString::fromLatin1(
            "SELECT COALESCE(MAX(id), 0) FROM %1;").arg(table)) ||
        !query.next())
    {
        return false;
    }

    *id = query.value(0).toInt();
    return true;
}

void reportProgress(const PrivacyProfileMerge::Progress& progress,
                    const QString& step,
                    int current,
                    int total)
{
    if (progress)
    {
        progress(step, current, total);
    }
}

} // namespace

PrivacyProfileMergeResult PrivacyProfileMerge::mergeStockIntoP1(
    const QString& convertedSourceP1Path,
    const QString& candidateP1Path,
    const Progress& progress,
    const IsCanceled& isCanceled)
{
    PrivacyProfileMergeResult result;
    const QString sourceConnection = connectionNameFor("privacy-merge-source-");
    const QString targetConnection = connectionNameFor("privacy-merge-target-");

    {
        QSqlDatabase source = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"),
                                                        sourceConnection);
        source.setConnectOptions(QLatin1String("QSQLITE_OPEN_READONLY"));
        source.setDatabaseName(convertedSourceP1Path);

        QSqlDatabase target = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"),
                                                        targetConnection);
        target.setDatabaseName(candidateP1Path);

        if (!source.open() || !target.open())
        {
            result.error = source.lastError().text().isEmpty()
                         ? target.lastError().text()
                         : source.lastError().text();
        }
        else if (!target.transaction())
        {
            result.error = target.lastError().text();
        }
        else
        {
            QHash<int, int> rootMap;
            QHash<int, int> albumMap;
            QHash<int, int> imageMap;
            QHash<int, int> tagMap;
            QHash<int, int> rootMaxId;
            int maxAlbumId = 0;
            int maxImageId = 0;
            int maxTagId = 0;
            int maxCommentId = 0;
            int maxCopyrightId = 0;

            const bool idsReady = nextId(target, QLatin1String("AlbumRoots"),
                                         &rootMaxId[0]) &&
                                  nextId(target, QLatin1String("Albums"), &maxAlbumId) &&
                                  nextId(target, QLatin1String("Images"), &maxImageId) &&
                                  nextId(target, QLatin1String("Tags"), &maxTagId) &&
                                  nextId(target, QLatin1String("ImageComments"),
                                         &maxCommentId) &&
                                  nextId(target, QLatin1String("ImageCopyright"),
                                         &maxCopyrightId);

            if (!idsReady)
            {
                result.error = QLatin1String(
                    "The additive merge could not allocate target row identifiers");
            }

            if (result.error.isEmpty())
            {
                // Existing target roots, keyed by specific path.
                QHash<QString, int> rootsByPath;
                QSqlQuery targetRoots(target);

                if (targetRoots.exec(QLatin1String(
                        "SELECT id, specificPath FROM AlbumRoots;")))
                {
                    while (targetRoots.next())
                    {
                        rootsByPath.insert(targetRoots.value(1).toString(),
                                           targetRoots.value(0).toInt());
                    }
                }
                else
                {
                    result.error = targetRoots.lastError().text();
                }

                QSqlQuery sourceRoots(source);

                if (result.error.isEmpty() &&
                    sourceRoots.exec(QLatin1String(
                        "SELECT id, label, status, type, identifier, specificPath, "
                        "caseSensitivity FROM AlbumRoots;")))
                {
                    QSqlQuery insertRoot(target);
                    insertRoot.prepare(QLatin1String(
                        "INSERT INTO AlbumRoots (id, label, status, type, identifier, "
                        "specificPath, caseSensitivity) VALUES (?, ?, ?, ?, ?, ?, ?);"));

                    while (sourceRoots.next())
                    {
                        const int sourceId = sourceRoots.value(0).toInt();
                        const QString specificPath = sourceRoots.value(5).toString();
                        const int existing = rootsByPath.value(specificPath, -1);

                        if (existing >= 0)
                        {
                            rootMap.insert(sourceId, existing);
                            continue;
                        }

                        const int newId = ++rootMaxId[0];
                        insertRoot.addBindValue(newId);
                        insertRoot.addBindValue(sourceRoots.value(1));
                        insertRoot.addBindValue(sourceRoots.value(2));
                        insertRoot.addBindValue(sourceRoots.value(3));
                        insertRoot.addBindValue(sourceRoots.value(4));
                        insertRoot.addBindValue(specificPath);
                        insertRoot.addBindValue(sourceRoots.value(6));

                        if (!insertRoot.exec())
                        {
                            result.error = insertRoot.lastError().text();
                            break;
                        }

                        rootsByPath.insert(specificPath, newId);
                        rootMap.insert(sourceId, newId);
                        ++result.addedRootCount;
                    }
                }

                QHash<QString, int> albumsByKey;
                QSqlQuery targetAlbums(target);

                if (result.error.isEmpty() &&
                    targetAlbums.exec(QLatin1String(
                        "SELECT id, albumRoot, relativePath FROM Albums;")))
                {
                    while (targetAlbums.next())
                    {
                        albumsByKey.insert(
                            QString::number(targetAlbums.value(1).toInt()) +
                            QLatin1Char('|') + targetAlbums.value(2).toString(),
                            targetAlbums.value(0).toInt());
                    }
                }
                else if (result.error.isEmpty())
                {
                    result.error = targetAlbums.lastError().text();
                }

                QSqlQuery sourceAlbums(source);

                if (result.error.isEmpty() &&
                    sourceAlbums.exec(QLatin1String(
                        "SELECT id, albumRoot, relativePath, date, caption, collection, "
                        "icon, modificationDate FROM Albums;")))
                {
                    QSqlQuery insertAlbum(target);
                    insertAlbum.prepare(QLatin1String(
                        "INSERT INTO Albums (id, albumRoot, relativePath, date, caption, "
                        "collection, icon, modificationDate) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);"));

                    while (sourceAlbums.next())
                    {
                        const int sourceId = sourceAlbums.value(0).toInt();
                        const int mappedRoot = rootMap.value(sourceAlbums.value(1).toInt(), -1);

                        if (mappedRoot < 0)
                        {
                            result.warnings << QString::fromLatin1(
                                "Source album %1 references an unknown root and was skipped")
                                                   .arg(sourceId);
                            continue;
                        }

                        const QString key = QString::number(mappedRoot) +
                                            QLatin1Char('|') +
                                            sourceAlbums.value(2).toString();
                        const int existing = albumsByKey.value(key, -1);

                        if (existing >= 0)
                        {
                            albumMap.insert(sourceId, existing);
                            continue;
                        }

                        const int newId = ++maxAlbumId;
                        insertAlbum.addBindValue(newId);
                        insertAlbum.addBindValue(mappedRoot);
                        insertAlbum.addBindValue(sourceAlbums.value(2));
                        insertAlbum.addBindValue(sourceAlbums.value(3));
                        insertAlbum.addBindValue(sourceAlbums.value(4));
                        insertAlbum.addBindValue(sourceAlbums.value(5));
                        insertAlbum.addBindValue(sourceAlbums.value(6));
                        insertAlbum.addBindValue(sourceAlbums.value(7));

                        if (!insertAlbum.exec())
                        {
                            result.error = insertAlbum.lastError().text();
                            break;
                        }

                        albumsByKey.insert(key, newId);
                        albumMap.insert(sourceId, newId);
                        ++result.addedAlbumCount;
                    }
                }

                QSqlQuery sourceImages(source);
                int sourceImageCount = 0;

                if (result.error.isEmpty() &&
                    sourceImages.exec(QLatin1String(
                        "SELECT COUNT(*) FROM Images;")))
                {
                    if (sourceImages.next())
                    {
                        sourceImageCount = sourceImages.value(0).toInt();
                    }
                }

                QSqlQuery existingImage(target);
                existingImage.prepare(QLatin1String(
                    "SELECT 1 FROM Images WHERE album=? AND name=?;"));
                QSqlQuery insertImage(target);
                insertImage.prepare(QLatin1String(
                    "INSERT INTO Images (id, album, name, status, category, "
                    "modificationDate, fileSize, uniqueHash, manualOrder) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);"));
                int processedImages = 0;

                if (result.error.isEmpty() &&
                    sourceImages.exec(QLatin1String(
                        "SELECT id, album, name, status, category, modificationDate, "
                        "fileSize, uniqueHash, manualOrder FROM Images;")))
                {
                    while (sourceImages.next())
                    {
                        if (isCanceled && isCanceled())
                        {
                            result.canceled = true;
                            result.error = QLatin1String(
                                "The additive catalogue merge was canceled");
                            break;
                        }

                        const int sourceId = sourceImages.value(0).toInt();
                        const int mappedAlbum = albumMap.value(
                            sourceImages.value(1).toInt(), -1);

                        if (mappedAlbum < 0)
                        {
                            result.warnings << QString::fromLatin1(
                                "Source image %1 references an unknown album and was skipped")
                                                   .arg(sourceId);
                            ++processedImages;
                            continue;
                        }

                        existingImage.addBindValue(mappedAlbum);
                        existingImage.addBindValue(sourceImages.value(2));

                        if (existingImage.exec() && existingImage.next())
                        {
                            ++result.skippedExistingItemCount;
                            existingImage.finish();
                            ++processedImages;
                            continue;
                        }

                        existingImage.finish();
                        const int newId = ++maxImageId;
                        insertImage.addBindValue(newId);
                        insertImage.addBindValue(mappedAlbum);
                        insertImage.addBindValue(sourceImages.value(2));
                        insertImage.addBindValue(sourceImages.value(3));
                        insertImage.addBindValue(sourceImages.value(4));
                        insertImage.addBindValue(sourceImages.value(5));
                        insertImage.addBindValue(sourceImages.value(6));
                        insertImage.addBindValue(sourceImages.value(7));
                        insertImage.addBindValue(sourceImages.value(8));

                        if (!insertImage.exec())
                        {
                            result.error = insertImage.lastError().text();
                            break;
                        }

                        imageMap.insert(sourceId, newId);
                        ++result.addedItemCount;
                        ++processedImages;

                        if ((processedImages % 500) == 0)
                        {
                            reportProgress(progress,
                                           QLatin1String("Merge catalogue items"),
                                           processedImages, sourceImageCount);
                        }
                    }
                }

                // Tags: parents normally precede children in id order, so a
                // single pass with a parent map is sufficient for real catalogues.
                QHash<QString, int> tagsByKey;
                QSqlQuery targetTags(target);

                if (result.error.isEmpty() &&
                    targetTags.exec(QLatin1String(
                        "SELECT id, pid, name FROM Tags;")))
                {
                    while (targetTags.next())
                    {
                        const int pid = targetTags.value(1).isNull()
                                      ? 0 : targetTags.value(1).toInt();
                        tagsByKey.insert(QString::number(pid) +
                                         QLatin1Char('|') +
                                         targetTags.value(2).toString(),
                                         targetTags.value(0).toInt());
                    }
                }
                else if (result.error.isEmpty())
                {
                    result.error = targetTags.lastError().text();
                }

                QSqlQuery sourceTags(source);

                if (result.error.isEmpty() &&
                    sourceTags.exec(QLatin1String(
                        "SELECT id, pid, name, icon, iconkde FROM Tags "
                        "ORDER BY id;")))
                {
                    QSqlQuery insertTag(target);
                    insertTag.prepare(QLatin1String(
                        "INSERT INTO Tags (id, pid, name, icon, iconkde) "
                        "VALUES (?, ?, ?, ?, ?);"));

                    while (sourceTags.next())
                    {
                        const int sourceId = sourceTags.value(0).toInt();
                        const int sourcePid = sourceTags.value(1).isNull()
                                            ? 0 : sourceTags.value(1).toInt();
                        const int mappedParent = (sourcePid == 0)
                                               ? 0 : tagMap.value(sourcePid, -1);
                        const QString key = QString::number(mappedParent) +
                                            QLatin1Char('|') +
                                            sourceTags.value(2).toString();

                        if ((sourcePid > 0) && (mappedParent < 0))
                        {
                            result.warnings << QString::fromLatin1(
                                "Source tag %1 has an unmapped parent and was skipped")
                                                   .arg(sourceId);
                            continue;
                        }

                        const int existing = tagsByKey.value(key, -1);

                        if (existing >= 0)
                        {
                            tagMap.insert(sourceId, existing);
                            continue;
                        }

                        const int newId = ++maxTagId;
                        insertTag.addBindValue(newId);
                        insertTag.addBindValue(mappedParent);
                        insertTag.addBindValue(sourceTags.value(2));
                        insertTag.addBindValue(sourceTags.value(3));
                        insertTag.addBindValue(sourceTags.value(4));

                        if (!insertTag.exec())
                        {
                            result.error = insertTag.lastError().text();
                            break;
                        }

                        tagsByKey.insert(key, newId);
                        tagMap.insert(sourceId, newId);
                        ++result.addedTagCount;
                    }
                }

                if (result.error.isEmpty() && !imageMap.isEmpty())
                {
                    QSqlQuery sourceTagsForImage(source);
                    sourceTagsForImage.prepare(QLatin1String(
                        "SELECT tagid FROM ImageTags WHERE imageid=?;"));
                    QSqlQuery insertImageTag(target);
                    insertImageTag.prepare(QLatin1String(
                        "INSERT OR IGNORE INTO ImageTags (imageid, tagid) "
                        "VALUES (?, ?);"));
                    QSqlQuery sourceInformation(source);
                    sourceInformation.prepare(QLatin1String(
                        "SELECT rating, creationDate, digitizationDate, orientation, width, "
                        "height, format, colorDepth, colorModel, timeZone "
                        "FROM ImageInformation WHERE imageid=?;"));
                    QSqlQuery insertInformation(target);
                    insertInformation.prepare(QLatin1String(
                        "INSERT INTO ImageInformation (imageid, rating, creationDate, "
                        "digitizationDate, orientation, width, height, format, colorDepth, "
                        "colorModel, timeZone) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"));
                    QSqlQuery sourceMetadata(source);
                    sourceMetadata.prepare(QLatin1String(
                        "SELECT make, model, lens, aperture, focalLength, focalLength35, "
                        "exposureTime, exposureProgram, exposureMode, sensitivity, flash, "
                        "whiteBalance, whiteBalanceColorTemperature, meteringMode, "
                        "subjectDistance, subjectDistanceCategory "
                        "FROM ImageMetadata WHERE imageid=?;"));
                    QSqlQuery insertMetadata(target);
                    insertMetadata.prepare(QLatin1String(
                        "INSERT INTO ImageMetadata (imageid, make, model, lens, aperture, "
                        "focalLength, focalLength35, exposureTime, exposureProgram, "
                        "exposureMode, sensitivity, flash, whiteBalance, "
                        "whiteBalanceColorTemperature, meteringMode, subjectDistance, "
                        "subjectDistanceCategory) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
                        "?, ?, ?, ?, ?, ?);"));
                    QSqlQuery sourceHistory(source);
                    sourceHistory.prepare(QLatin1String(
                        "SELECT uuid, history FROM ImageHistory WHERE imageid=?;"));
                    QSqlQuery insertHistory(target);
                    insertHistory.prepare(QLatin1String(
                        "INSERT INTO ImageHistory (imageid, uuid, history) "
                        "VALUES (?, ?, ?);"));
                    QSqlQuery sourceProperties(source);
                    sourceProperties.prepare(QLatin1String(
                        "SELECT property, value FROM ImageProperties WHERE imageid=?;"));
                    QSqlQuery insertProperty(target);
                    insertProperty.prepare(QLatin1String(
                        "INSERT OR IGNORE INTO ImageProperties (imageid, property, value) "
                        "VALUES (?, ?, ?);"));
                    QSqlQuery sourceComments(source);
                    sourceComments.prepare(QLatin1String(
                        "SELECT type, language, author, date, comment FROM ImageComments "
                        "WHERE imageid=?;"));
                    QSqlQuery insertComment(target);
                    insertComment.prepare(QLatin1String(
                        "INSERT INTO ImageComments (id, imageid, type, language, author, "
                        "date, comment) VALUES (?, ?, ?, ?, ?, ?, ?);"));
                    QSqlQuery sourceCopyright(source);
                    sourceCopyright.prepare(QLatin1String(
                        "SELECT property, value, extraValue FROM ImageCopyright "
                        "WHERE imageid=?;"));
                    QSqlQuery insertCopyright(target);
                    insertCopyright.prepare(QLatin1String(
                        "INSERT INTO ImageCopyright (id, imageid, property, value, "
                        "extraValue) VALUES (?, ?, ?, ?, ?, ?);"));

                    int copiedMetadata = 0;

                    for (auto it = imageMap.cbegin();
                         result.error.isEmpty() && (it != imageMap.cend()); ++it)
                    {
                        const int sourceImageId = it.key();
                        const int targetImageId = it.value();

                        sourceTagsForImage.addBindValue(sourceImageId);

                        if (sourceTagsForImage.exec())
                        {
                            while (sourceTagsForImage.next())
                            {
                                const int mappedTag = tagMap.value(
                                    sourceTagsForImage.value(0).toInt(), -1);

                                if (mappedTag < 0)
                                {
                                    continue;
                                }

                                insertImageTag.addBindValue(targetImageId);
                                insertImageTag.addBindValue(mappedTag);
                                insertImageTag.exec();
                                insertImageTag.finish();
                                ++copiedMetadata;
                            }
                        }

                        sourceTagsForImage.finish();
                        sourceInformation.addBindValue(sourceImageId);

                        if (sourceInformation.exec() && sourceInformation.next())
                        {
                            insertInformation.addBindValue(targetImageId);

                            for (int column = 0; column < 10; ++column)
                            {
                                insertInformation.addBindValue(
                                    sourceInformation.value(column));
                            }

                            if (insertInformation.exec())
                            {
                                ++copiedMetadata;
                            }

                            insertInformation.finish();
                        }

                        sourceInformation.finish();
                        sourceMetadata.addBindValue(sourceImageId);

                        if (sourceMetadata.exec() && sourceMetadata.next())
                        {
                            insertMetadata.addBindValue(targetImageId);

                            for (int column = 0; column < 16; ++column)
                            {
                                insertMetadata.addBindValue(sourceMetadata.value(column));
                            }

                            if (insertMetadata.exec())
                            {
                                ++copiedMetadata;
                            }

                            insertMetadata.finish();
                        }

                        sourceMetadata.finish();
                        sourceHistory.addBindValue(sourceImageId);

                        if (sourceHistory.exec() && sourceHistory.next())
                        {
                            insertHistory.addBindValue(targetImageId);
                            insertHistory.addBindValue(sourceHistory.value(0));
                            insertHistory.addBindValue(sourceHistory.value(1));

                            if (insertHistory.exec())
                            {
                                ++copiedMetadata;
                            }

                            insertHistory.finish();
                        }

                        sourceHistory.finish();
                        sourceProperties.addBindValue(sourceImageId);

                        if (sourceProperties.exec())
                        {
                            while (sourceProperties.next())
                            {
                                insertProperty.addBindValue(targetImageId);
                                insertProperty.addBindValue(sourceProperties.value(0));
                                insertProperty.addBindValue(sourceProperties.value(1));

                                if (insertProperty.exec())
                                {
                                    ++copiedMetadata;
                                }

                                insertProperty.finish();
                            }
                        }

                        sourceProperties.finish();
                        sourceComments.addBindValue(sourceImageId);

                        if (sourceComments.exec())
                        {
                            while (sourceComments.next())
                            {
                                insertComment.addBindValue(++maxCommentId);
                                insertComment.addBindValue(targetImageId);
                                insertComment.addBindValue(sourceComments.value(0));
                                insertComment.addBindValue(sourceComments.value(1));
                                insertComment.addBindValue(sourceComments.value(2));
                                insertComment.addBindValue(sourceComments.value(3));
                                insertComment.addBindValue(sourceComments.value(4));

                                if (insertComment.exec())
                                {
                                    ++copiedMetadata;
                                }

                                insertComment.finish();
                            }
                        }

                        sourceComments.finish();
                        sourceCopyright.addBindValue(sourceImageId);

                        if (sourceCopyright.exec())
                        {
                            while (sourceCopyright.next())
                            {
                                insertCopyright.addBindValue(++maxCopyrightId);
                                insertCopyright.addBindValue(targetImageId);
                                insertCopyright.addBindValue(sourceCopyright.value(0));
                                insertCopyright.addBindValue(sourceCopyright.value(1));
                                insertCopyright.addBindValue(sourceCopyright.value(2));

                                if (insertCopyright.exec())
                                {
                                    ++copiedMetadata;
                                }

                                insertCopyright.finish();
                            }
                        }

                        sourceCopyright.finish();
                        result.copiedMetadataRowCount = copiedMetadata;
                    }
                }
            }

            if (result.error.isEmpty())
            {
                if (!target.commit())
                {
                    result.error = target.lastError().text();
                }
            }
            else
            {
                target.rollback();
            }

            source.close();
            target.close();
        }
    }

    QSqlDatabase::removeDatabase(sourceConnection);
    QSqlDatabase::removeDatabase(targetConnection);
    result.success = result.error.isEmpty();
    return result;
}

} // namespace Digikam
