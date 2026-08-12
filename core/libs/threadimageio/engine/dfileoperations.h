/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : misc file operation methods
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QString>
#include <QDateTime>
#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DFileOperations
{

public:

    enum SidecarAction
    {
        Rename = 0,
        Copy
    };

public:

    /**
     * This method rename a local file 'orgPath' to 'destPath' with all ACL properties
     * restoration taken from 'source' file.
     * Return true if operation is completed.
     */
    static bool localFileRename(const QString& source,
                                const QString& orgPath,
                                const QString& destPath,
                                bool ignoreSettings = false);

    /**
     * Open file urls to default application relevant of
     * file type-mimes desktop configuration.
     */
    static void openFilesWithDefaultApplication(const QList<QUrl>& urls);

    /**
     * Get unique file url if file exist by appending a counter suffix
     * or return original url.
     */
    static QUrl getUniqueFileUrl(const QUrl& orgUrl, bool* const newurl = nullptr);

    /**
     * Get unique folder url if folder exist by appending a counter suffix
     * or return original url.
     */
    static QUrl getUniqueFolderUrl(const QUrl& orgUrl);

    /**
     * Open system file manager and select the item.
     */
    static void openInFileManager(const QList<QUrl>& urls);

    /**
     * Copy recursively a directory contents to another one.
     */
    static bool copyFolderRecursively(const QString& srcPath,
                                      const QString& dstPath,
                                      const QString& itemId = QString(),
                                      bool* const cancel = nullptr,
                                      bool useDstPath = false);

    /**
     * Copy a list of files to another place.
     */
    static bool copyFiles(const QStringList& srcPaths,
                          const QString& dstPath);

    /**
     * Rename or move file and keep the source file modification time.
     */
    static bool renameFile(const QString& srcFile,
                           const QString& dstFile);

    /**
     * Copy file and keep the source file modification time.
     */
    static bool copyFile(const QString& srcFile,
                         const QString& dstFile,
                         const bool* const cancel = nullptr);

    /**
     * If the destination file already exists,
     * it will be removed. Copy file and keep
     * the source file modification time.
     */
    static bool removeAndCopyFile(const QString& srcFile,
                                  const QString& dstFile);

    /**
     * Copy file modification time from source to destination file.
     */
    static bool copyModificationTime(const QString& srcFile,
                                     const QString& dstFile);
    /**
     * Set file modification time from QDateTime.
     * Keep access time from source file.
     */
    static bool setModificationTime(const QString& srcFile,
                                    const QDateTime& dateTime);
    /**
     * Returns the path to a program under Windows by searching
     * in the Windows registry.
     * If the path is empty, QStandardPaths::findExecutable()
     * is used as under Linux and macOS.
     */
    static QString findExecutable(const QString& name,
                                  const QStringList& hints = QStringList());

    /**
     * Rename/move or copy all possible sidecar files
     * and keep the source file modification time.
     */
    static bool sidecarFiles(const QString& srcFile,
                             const QString& dstFile,
                             SidecarAction action);
};

} // namespace Digikam
