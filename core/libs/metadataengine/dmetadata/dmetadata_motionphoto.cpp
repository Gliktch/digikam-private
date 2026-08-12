/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-03-25
 * Description : item metadata interface - Motion Photo helpers
 *
 * SPDX-FileCopyrightText: 2026 by Robert Siebeck <robert at siebeck dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Qt includes

#include <QScopedPointer>

// Local includes

#include "digikam_debug.h"
#include "exiftoolparser.h"

namespace Digikam
{

bool DMetadata::isMotionPhoto() const
{
    if (
        (getXmpTagString("Xmp.GCamera.MotionPhoto") == QLatin1String("1")) ||
        (getXmpTagString("Xmp.GCamera.MicroVideo")  == QLatin1String("1"))
       )
    {
        return true;
    }

    return false;
}

QByteArray DMetadata::extractMotionPhotoVideo() const
{
    QScopedPointer<ExifToolParser> parser(new ExifToolParser(nullptr));

    if (!parser->exifToolAvailable())
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifTool not available, cannot extract motion photo video";

        return QByteArray();
    }

    QString filePath = getFilePath();

    if (!parser->extractMotionPhotoVideo(filePath))
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifTool failed to extract motion photo video from" << filePath;

        return QByteArray();
    }

    QByteArray data = parser->currentData().value(QLatin1String("MOTION_PHOTO_VIDEO")).at(0).toByteArray();

    if (data.isEmpty())
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "No motion photo video found in" << filePath;

        return QByteArray();
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "Extracted motion photo video"
                                    << "(" << data.size() << "bytes) from" << filePath;

    return data;
}

} // namespace Digikam
