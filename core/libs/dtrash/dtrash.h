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

#pragma once

// Qt includes

#include <QObject>
#include <QFileInfo>

// Local includes

#include "dtrashiteminfo.h"

namespace Digikam
{

class DTrash
{

public:

    const static QString TRASH_FOLDER;
    const static QString FILES_FOLDER;
    const static QString INFO_FOLDER;
    const static QString INFO_FILE_EXTENSION;
    const static QString PATH_JSON_KEY;
    const static QString DELETIONTIMESTAMP_JSON_KEY;
    const static QString IMAGEID_JSON_KEY;

public:

    /**
     * @brief Deletes image to the trash of a particular collection
     *
     * @param imagePath: path to image
     * @param deleteTime: delete time from the image
     *
     * @return true if the image was deleted
     */
    static bool deleteImage(const QString& imagePath, const QDateTime& deleteTime);

    /**
     * @brief Deletes a whole folder from the collection
     *
     * @param dirToDelete: path to folder
     * @param deleteTime: delete time from the image
     *
     * @return true if folder was deleted
     */
    static bool deleteDirRecursivley(const QString& dirToDelete, const QDateTime& deleteTime);

    /**
     * @brief Extracts the data from json file and gives it to DTrashItemInfo
     *
     * @param collPath: path to collection
     * @param baseName: name of the file inside the trash
     * @param itemInfo: item to extract data to it
     */
    static void extractJsonForItem(const QString& collPath, const QString& baseName, DTrashItemInfo& itemInfo);

private:

    /**
     * @brief Ensures that the trash caching folder exists before moving items to trash
     *
     * @param collectionPath: the full path to the collection to prepare the trash for
     *
     * @return true if the trash is prepared successfully
     */
    static bool prepareCollectionTrash(const QString& collectionPath);

    /**
     * @brief Creates a json file containing the image path and deletion timestamp
     *        and return the baseName for this json file
     *
     * @param imageId: the image id for the file
     * @param imagePath: path of image to create json file for
     * @param deleteTime: delete time from the image
     * @param collectionPath: path of collection that the image belongs to
     *
     * Usage: createJsonRecordForFile("home/user/Pics", "/home/user/Pics/cats/cute/katy.jpg")
     *          returns => "katy"
     */
    static QString createJsonRecordForFile(qlonglong imageId,
                                           const QString& imagePath,
                                           const QDateTime& deleteTime,
                                           const QString& collectionPath);

    /**
     * @brief Checks for duplicates of files names inside the trash and if there is
     *        a duplication it does a simple versioning recursively
     *
     * @param collectionPath: path of collection that the image belongs to
     * @param baseName: the baseName of the image
     * @param version: a digit to append to the image baseName
     *
     * Usage: getAvialableJsonFilePathInTrash("home/user/Pics", "katy", 0)
     *          return => "home/user/Pics/.trash/info/katy.dtrashInfo"
     *          if this name was available, if not
     *          returns => "home/user/Pics/.trash/info/katy{version}.dtrashInfo"
     *          where {version} is a integer number that is available in trash
     */
    static QString getAvialableJsonFilePathInTrash(const QString& collectionPath,
                                                   const QString& baseName, int version = 0);

private:

    DTrash() = default;
};

} // namespace Digikam
