/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG - Post process with ExifTool.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngwriter_p.h"

namespace Digikam
{

int DNGWriter::Private::exifToolPostProcess()
{
    QScopedPointer<ExifToolParser> const parser(new ExifToolParser(nullptr));

    if (parser->exifToolAvailable())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Post-process Iptc and Xmp with ExifTool";

        bool ret = parser->copyTags(
                                     inputInfo.filePath(),
                                     dngFilePath,
                                     ExifToolProcess::COPY_IPTC         |
                                     ExifToolProcess::COPY_XMP,
                                     ExifToolProcess::CREATE_NEW_TAGS   |
                                     ExifToolProcess::CREATE_NEW_GROUPS
                                   );

        if (!ret)
        {
            qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Copy Iptc and Xmp tags with ExifTool failed. Aborted...";

            return PROCESS_FAILED;
        }

        // ---

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Post-process markernotes with ExifTool";

        ret      = parser->copyTags(
                                     inputInfo.filePath(),
                                     dngFilePath,
                                     ExifToolProcess::COPY_MAKERNOTES |
                                     ExifToolProcess::RESTORE_PREVIEW
                                   );

        if (!ret)
        {
            qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Copy markernotes with ExifTool failed. Aborted...";

            return PROCESS_FAILED;
        }

        // ---

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Populate Xmp with ExifTool";

        ret      = parser->translateTags(
                                          dngFilePath,
                                          ExifToolProcess::TRANS_ALL_XMP
                                        );

        if (!ret)
        {
            qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Populate Xmp tags with ExifTool failed. Aborted...";

            return PROCESS_FAILED;
        }

        // ---

        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Populate Iptc with ExifTool";

        ret      = parser->translateTags(
                                          dngFilePath,
                                          ExifToolProcess::TRANS_ALL_IPTC
                                        );

        if (!ret)
        {
            qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Populate Iptc tags with ExifTool failed. Aborted...";

            return PROCESS_FAILED;
        }
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "DNGWriter: ExifTool is not available to post-process metadata...";
    }

    return PROCESS_CONTINUE;
}

} // namespace Digikam
