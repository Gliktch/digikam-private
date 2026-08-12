/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-07-27
 * Description : Special digiKam trash implementation
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dtrash.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QUuid>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QDateTime>

// Local includes

#include "digikam_debug.h"
#include "collectionmanager.h"
#include "albummanager.h"

namespace Digikam
{

const QString DTrash::TRASH_FOLDER               = QLatin1String(".dtrash");
const QString DTrash::FILES_FOLDER               = QLatin1String("files");
const QString DTrash::INFO_FOLDER                = QLatin1String("info");
const QString DTrash::INFO_FILE_EXTENSION        = QLatin1String(".dtrashinfo");
const QString DTrash::PATH_JSON_KEY              = QLatin1String("path");
const QString DTrash::DELETIONTIMESTAMP_JSON_KEY = QLatin1String("deletiontimestamp");
const QString DTrash::IMAGEID_JSON_KEY           = QLatin1String("imageid");

// ----------------------------------------------

bool DTrash::deleteImage(const QString& imagePath, const QDateTime& deleteTime)
{
    QString collection = CollectionManager::instance()->albumRootPath(imagePath);

    qCDebug(DIGIKAM_IOJOB_LOG)  << "DTrash: Image album root path:"
                                << collection;

    if (!prepareCollectionTrash(collection))
    {
        return false;
    }

    QFileInfo imageFileInfo(imagePath);
    QString fileName     = imageFileInfo.fileName();

    // Get the album path, i.e. collection + album. For this,
    // get the n leftmost characters where n is the complete path without the size of the filename

    QString completePath = imageFileInfo.path();

    qlonglong imageId    = -1;

    // Get the album and with this the image id of the image to trash.

    PAlbum* const pAlbum = AlbumManager::instance()->findPAlbum(QUrl::fromLocalFile(completePath));

    if (pAlbum)
    {
        imageId = AlbumManager::instance()->getItemFromAlbum(pAlbum, fileName);
    }

    QString baseNameForMovingIntoTrash = createJsonRecordForFile(imageId,
                                                                 imagePath,
                                                                 deleteTime,
                                                                 collection);

    QString destinationInTrash = collection + QLatin1Char('/') + TRASH_FOLDER       +
                                 QLatin1Char('/') + FILES_FOLDER + QLatin1Char('/') +
                                 baseNameForMovingIntoTrash + QLatin1Char('.')      +
                                 imageFileInfo.completeSuffix();

    if (!QFile::rename(imagePath, destinationInTrash))
    {
        return false;
    }

    return true;
}

bool DTrash::deleteDirRecursivley(const QString& dirToDelete, const QDateTime& deleteTime)
{
    QDir srcDir(dirToDelete);
    const auto list = srcDir.entryInfoList(QDir::Files);

    for (const QFileInfo& fileInfo : list)
    {
        if (!deleteImage(fileInfo.filePath(), deleteTime))
        {   // cppcheck-suppress useStlAlgorithm
            return false;
        }
    }

    const auto list2 = srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& fileInfo : list2)
    {
        if (!deleteDirRecursivley(fileInfo.filePath(), deleteTime))
        {   // cppcheck-suppress useStlAlgorithm
            return false;
        }
    }

    return srcDir.removeRecursively();
}

void DTrash::extractJsonForItem(const QString& collPath, const QString& baseName, DTrashItemInfo& itemInfo)
{
    QString jsonFilePath = collPath + QLatin1Char('/') + TRASH_FOLDER        +
                           QLatin1Char('/') + INFO_FOLDER + QLatin1Char('/') +
                           baseName + INFO_FILE_EXTENSION;

    QFile jsonFile(jsonFilePath);

    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QJsonDocument doc               = QJsonDocument::fromJson(jsonFile.readAll());
    jsonFile.close();

    QJsonObject fileInfoObj         = doc.object();

    itemInfo.jsonFilePath           = jsonFilePath;

    itemInfo.collectionPath         = fileInfoObj.value(PATH_JSON_KEY).toString();

    itemInfo.collectionRelativePath = fileInfoObj.value(PATH_JSON_KEY).toString()
                                      .replace(collPath, QLatin1String(""));

    itemInfo.deletionTimestamp      = QDateTime::fromString(
                                      fileInfoObj.value(DELETIONTIMESTAMP_JSON_KEY).toString(), Qt::ISODate);

    if (!itemInfo.deletionTimestamp.isValid())
    {
        // Failback to date encoded as string using locale.
        // This is an older way to store date in JSOn, which do not support change in locale.
        // This is why ISO format is now used.

        itemInfo.deletionTimestamp  = QDateTime::fromString(
                                      fileInfoObj.value(DELETIONTIMESTAMP_JSON_KEY).toString());
    }

    QJsonValue imageIdValue         = fileInfoObj.value(IMAGEID_JSON_KEY);

    if (!imageIdValue.isUndefined())
    {
        itemInfo.imageId = imageIdValue.toString().toLongLong();
    }
    else
    {
        itemInfo.imageId = -1;
    }
}

bool DTrash::prepareCollectionTrash(const QString& collectionPath)
{
    QString trashFolder = collectionPath + QLatin1Char('/') + TRASH_FOLDER;
    QString trashFiles  = trashFolder    + QLatin1Char('/') + FILES_FOLDER;
    QString trashInfo   = trashFolder    + QLatin1Char('/') + INFO_FOLDER;
    bool    isCreated   = !collectionPath.isEmpty();

    if (isCreated && !QFileInfo::exists(trashFolder))
    {
        isCreated &= QDir().mkpath(trashFolder);
    }

    if (isCreated && !QFileInfo::exists(trashFiles))
    {
        isCreated &= QDir().mkpath(trashFiles);
    }

    if (isCreated && !QFileInfo::exists(trashInfo))
    {
        isCreated &= QDir().mkpath(trashInfo);
    }

    if (!isCreated)
    {
        qCDebug(DIGIKAM_IOJOB_LOG) << "DTrash: could not create trash folder for collection";

        return false;
    }

    qCDebug(DIGIKAM_IOJOB_LOG) << "Trash folder for collection:" << trashFolder;

    return true;
}

QString DTrash::createJsonRecordForFile(qlonglong imageId,
                                        const QString& imagePath,
                                        const QDateTime& deleteTime,
                                        const QString& collectionPath)
{
    QJsonObject jsonObjForImg;

    QJsonValue pathJsonVal(imagePath);
    QJsonValue timestampJsonVal(deleteTime.toString(Qt::ISODate));
    QJsonValue imageIdJsonVal(QString::number(imageId));

    jsonObjForImg.insert(PATH_JSON_KEY, pathJsonVal);
    jsonObjForImg.insert(DELETIONTIMESTAMP_JSON_KEY, timestampJsonVal);
    jsonObjForImg.insert(IMAGEID_JSON_KEY, imageIdJsonVal);

    QJsonDocument jsonDocForImg(jsonObjForImg);

    QFileInfo imgFileInfo(imagePath);

    QString jsonFileName = getAvialableJsonFilePathInTrash(collectionPath,
                                                           imgFileInfo.baseName());

    QFile jsonFileForImg(jsonFileName);

    QFileInfo jsonFileInfo(jsonFileName);

    if (!jsonFileForImg.open(QFile::WriteOnly))
    {
        return jsonFileInfo.baseName();
    }

    jsonFileForImg.write(jsonDocForImg.toJson());
    jsonFileForImg.close();
    jsonFileForImg.setPermissions(QFileDevice::ReadOwner  |
                                  QFileDevice::ReadGroup  |
                                  QFileDevice::ReadOther  |
                                  QFileDevice::WriteOwner |
                                  QFileDevice::WriteGroup);

    return jsonFileInfo.baseName();
}

QString DTrash::getAvialableJsonFilePathInTrash(const QString& collectionPath,
                                                const QString& baseName,
                                                int version)
{
    QString pathToCreateJsonFile = collectionPath + QLatin1Char('/')                        +
                                   TRASH_FOLDER + QLatin1Char('/')                          +
                                   INFO_FOLDER + QLatin1Char('/')                           +
                                   baseName + QLatin1Char('-')                              +
                                   QUuid::createUuid().toString().mid(1, 8)                 +
                                   (version ? QString::number(version) : QLatin1String("")) +
                                   INFO_FILE_EXTENSION;

    QFileInfo jsonFileInfo(pathToCreateJsonFile);

    if (jsonFileInfo.exists())
    {
        return getAvialableJsonFilePathInTrash(collectionPath, baseName, ++version);
    }
    else
    {
        return pathToCreateJsonFile;
    }
}

} // namespace Digikam
