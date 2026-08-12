/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-20
 * Description : description of actions when saving a file with versioning
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "versionfileoperation.h"

// Qt includes

#include <QDir>
#include <QUrl>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

bool VersionFileInfo::isNull() const
{
    return fileName.isNull();
}

QString VersionFileInfo::filePath() const
{
    return (path + QLatin1Char('/') + fileName);
}

QUrl VersionFileInfo::fileUrl() const
{
    QUrl url = QUrl::fromLocalFile(path);
    url      = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + fileName);

    return url;
}

QStringList VersionFileOperation::allFilePaths() const
{
    QStringList paths;

    if (!saveFile.isNull())
    {
        paths << saveFile.filePath();
    }

    if (!intermediateForLoadedFile.isNull())
    {
        paths << intermediateForLoadedFile.filePath();
    }

    for (const VersionFileInfo& intermediate : std::as_const(intermediates))
    {
        paths << intermediate.filePath();
    }

    return paths;
}

} // namespace Digikam
