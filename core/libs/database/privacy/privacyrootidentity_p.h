/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

#ifdef Q_OS_UNIX

// POSIX includes

#   include <sys/stat.h>
#   include <sys/types.h>

#endif

namespace Digikam
{

namespace PrivacyRootIdentityInternal
{

#ifdef Q_OS_UNIX

inline QString filesystemUuidForDevice(dev_t device)
{
    const QDir uuidDirectory(QLatin1String("/dev/disk/by-uuid"));

    for (const QFileInfo& entry : uuidDirectory.entryInfoList(
             QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System,
             QDir::Name))
    {
        struct stat targetStat = {};
        const QByteArray targetPath = QFile::encodeName(entry.canonicalFilePath());

        if (!targetPath.isEmpty() && (::stat(targetPath.constData(), &targetStat) == 0) &&
            S_ISBLK(targetStat.st_mode) && (targetStat.st_rdev == device))
        {
            return QLatin1String("filesystem-uuid-v1:") + entry.fileName();
        }
    }

    return QString();
}

#endif

} // namespace PrivacyRootIdentityInternal

} // namespace Digikam
