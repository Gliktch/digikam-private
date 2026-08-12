/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - Exiftool helpers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Leif Huhn <leif at dkstat dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Qt includes

#include <QString>
#include <QFileInfo>
#include <QScopedPointer>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "exiftoolparser.h"

namespace Digikam
{

bool DMetadata::loadUsingExifTool(const QString& filePath, bool videoAll)
{
    QFileInfo info(filePath);

    const bool metaAll = (
                          (info.suffix().toUpper() == QLatin1String("EXR")) ||
                          (info.suffix().toUpper() == QLatin1String("FITS"))
                         );

    QScopedPointer<ExifToolParser> const parser(new ExifToolParser(nullptr));

    if (!parser->exifToolAvailable())
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifTool is not available to load metadata...";

        return false;
    }

    int loop = 0;

    while (loop < 2)
    {
        if (loop == 1)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Metadata chunk loading now without MakerNotes";
        }

        if (!parser->loadChunk(filePath, (videoAll || metaAll), (loop == 1)))
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "Load metadata using ExifTool failed...";

            return false;
        }

        ExifToolParser::ExifToolData chunk = parser->currentData();

        qCDebug(DIGIKAM_METAENGINE_LOG) << "Metadata chunk loaded with ExifTool";

        ExifToolParser::ExifToolData::iterator it = chunk.find(QLatin1String("EXV"));

        if (it == chunk.end())
        {
            qCWarning(DIGIKAM_METAENGINE_LOG) << "Metadata chunk loaded with ExifTool is empty";

            return false;
        }

        QVariantList varLst = it.value();
        QByteArray exv      = varLst[0].toByteArray();

        if (exv.isEmpty())
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Metadata chunk loaded with ExifTool has no data";

            return false;
        }

        ++loop;

        if (loadFromData(exv))
        {
            break;
        }
    }

    // Restore file path.

    setFilePath(filePath);

    return true;
}

} // namespace Digikam
