/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - file I/O helpers.
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
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QUuid>
#include <QMimeDatabase>

// Local includes

#include "filereadwritelock.h"
#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::load(const QString& filePath, bool videoAll, Backend* backend)
{
    FileReadLocker lock(filePath);

    Backend usedBackend = NoBackend;
    bool sidecarLoaded  = false;
    bool hasLoaded      = false;
    QFileInfo info(filePath);
    QMimeDatabase mimeDB;

    if (
        !mimeDB.mimeTypeForFile(info).name().startsWith(QLatin1String("video/")) &&
        !mimeDB.mimeTypeForFile(info).name().startsWith(QLatin1String("audio/")) &&
        (info.suffix().toUpper() != QLatin1String("INSV"))                       &&
        (info.suffix().toUpper() != QLatin1String("H264"))
       )
    {
        // Process images only with Exiv2 backend first, or Exiftool in 2nd, or libraw for RAW files,
        // or with libheif, or at end with ImageMagick.
        // Never process video files with Exiv2, the backend is very unstable and this feature will be removed.

        if (!(hasLoaded = MetaEngine::load(filePath)))
        {
            if (!(hasLoaded = loadUsingExifTool(filePath)))
            {
                if (!(hasLoaded = loadUsingRawEngine(filePath)))
                {

                    if (!(hasLoaded =
#ifdef HAVE_HEIF
                        loadUsingLibheif(filePath)
#else
                        false
#endif
                       ))
                    {
                        // cppcheck-suppress knownConditionTrueFalse
                        if (!(hasLoaded = false/*loadUsingImageMagick(filePath)*/))
                        {
                            usedBackend = NoBackend;
                        }
                        else
                        {
                            usedBackend = ImageMagickBackend;
                        }
                    }
                    else
                    {
                        usedBackend = LibHeifBackend;
                    }
                }
                else
                {
                    usedBackend = LibRawBackend;
                }
            }
            else
            {
                usedBackend = ExifToolBackend;
            }
        }
        else
        {
            // Special case when Exiv2 has empty metadata container
            // but no loading error, give ExifTool a chance.

            if (isEmpty() && (loadUsingExifTool(filePath)))
            {
                usedBackend = ExifToolBackend;
            }
            else
            {
                usedBackend = Exiv2Backend;
            }
        }

        sidecarLoaded = loadFromSidecarAndMerge(filePath);
    }
    else
    {
        // No image files (aka video or audio), process with ExifTool and ffmpeg backends.

        if ((hasLoaded = loadUsingExifTool(filePath, videoAll)))
        {
            // Load and merge metadata from sidecar first
            // before adding virtual metadata to XMP.

            sidecarLoaded = loadFromSidecarAndMerge(filePath);

            if (videoAll && (hasLoaded = loadUsingFFmpeg(filePath)))
            {
                usedBackend = VideoMergeBackend;
            }
            else
            {
                usedBackend = ExifToolBackend;
            }
        }
        else
        {
            sidecarLoaded = loadFromSidecarAndMerge(filePath);

            if (videoAll && (hasLoaded = loadUsingFFmpeg(filePath)))
            {
                usedBackend = FFMpegBackend;
            }
            else
            {
                usedBackend = NoBackend;
            }
        }
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "Loading metadata with"
                                    << backendName(usedBackend)
                                    << "backend from" << filePath;

    if (backend)
    {
        *backend = usedBackend;
    }

    return (hasLoaded || sidecarLoaded);
}

bool DMetadata::save(const QString& filePath, bool setVersion) const
{
    FileWriteLocker lock(filePath);

    return MetaEngine::save(filePath, setVersion);
}

bool DMetadata::applyChanges(bool setVersion) const
{
    FileWriteLocker lock(getFilePath());

    return MetaEngine::applyChanges(setVersion);
}

} // namespace Digikam
