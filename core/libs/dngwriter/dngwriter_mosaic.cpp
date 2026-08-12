/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG - Mosaic identification.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Jens Mueller <tschenser at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngwriter_p.h"

// Local includes

#include "dngwriterhost.h"

namespace Digikam
{

int DNGWriter::Private::identMosaic(DRawInfo* const identify,
                                    DRawInfo* const identifyMake)
{
    // Check if CFA layout is supported by DNG SDK.

    bool fujiRotate90 = false;

    // Standard bayer layouts

    if      (identify->filterPattern       == QLatin1String("GRBGGRBGGRBGGRBG"))
    {
        bayerPattern = Private::Standard;
        filter       = 0;
    }
    else if (identify->filterPattern       == QLatin1String("RGGBRGGBRGGBRGGB"))
    {
        bayerPattern = Private::Standard;
        filter       = 1;
    }
    else if (identify->filterPattern       == QLatin1String("BGGRBGGRBGGRBGGR"))
    {
        bayerPattern = Private::Standard;
        filter       = 2;
    }
    else if (identify->filterPattern       == QLatin1String("GBRGGBRGGBRGGBRG"))
    {
        bayerPattern = Private::Standard;
        filter       = 3;
    }
    else if ((identify->filterPattern      == QLatin1String("RGBGRGBGRGBGRGBG")) &&
             (identifyMake->make.toUpper() == QLatin1String("FUJIFILM")))
    {
        // Fuji layouts

        bayerPattern = Private::Fuji;
        fujiRotate90 = false;
        filter       = 0;
    }
    else if ((identify->filterPattern      == QLatin1String("RBGGBRGGRBGGBRGG")) &&
             (identifyMake->make.toUpper() == QLatin1String("FUJIFILM")))
    {
        // Fuji layouts

        bayerPattern = Private::Fuji;
        fujiRotate90 = true;
        filter       = 0;
    }
    else if ((identify->filterPattern      == QLatin1String("GGGGBRGGGGRBGGGG")) &&
             (identifyMake->make.toUpper() == QLatin1String("FUJIFILM")))
    {
        // Fuji layouts

        bayerPattern = Private::Fuji6x6;
        fujiRotate90 = false;
        filter       = 1;
    }
    else if ((identify->rawColors == 3)                 &&
             (identify->filterPattern.isEmpty())        &&
             ((uint32)rawData.size() == (identify->outputSize.width() * identify->outputSize.height() * 3 * sizeof(uint16))))
    {
        bayerPattern = Private::LinearRaw;
    }
    else if (identify->rawColors == 4)           // Four color sensors
    {
        bayerPattern = Private::FourColor;

        if (identify->filterPattern.length() != 16)
        {
            qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Bayer mosaic not supported. Aborted...";

            return FILE_NOT_SUPPORTED;
        }

        for (int i = 0 ; i < 16 ; ++i)
        {
            filter = filter >> 2;

            if      (identify->filterPattern[i] == QLatin1Char('G'))
            {
                filter |= 0x00000000;
            }
            else if (identify->filterPattern[i] == QLatin1Char('M'))
            {
                filter |= 0x40000000;
            }
            else if (identify->filterPattern[i] == QLatin1Char('C'))
            {
                filter |= 0x80000000;
            }
            else if (identify->filterPattern[i] == QLatin1Char('Y'))
            {
                filter |= 0xC0000000;
            }
            else
            {
                qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Bayer mosaic not supported. Aborted...";

                return FILE_NOT_SUPPORTED;
            }
        }
    }
    else
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Bayer mosaic not supported. Aborted...";

        return FILE_NOT_SUPPORTED;
    }

    if (fujiRotate90)
    {
        if (!fujiRotate(rawData, *identify))
        {
            qCCritical(DIGIKAM_GENERAL_LOG) << "Can not rotate fuji image. Aborted...";

            return PROCESS_FAILED;
        }

        int tmp      = outputWidth;
        outputWidth  = outputHeight;
        outputHeight = tmp;
    }

    activeArea   = dng_rect(identify->outputSize.height(), identify->outputSize.width());
    activeWidth  = identify->outputSize.width();
    activeHeight = identify->outputSize.height();

    // Check if number of Raw Color components is supported.

    if ((identify->rawColors != 3) &&
        (identify->rawColors != 4))
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Number of Raw color components not supported. Aborted...";

        return PROCESS_FAILED;
    }

    width  = identify->outputSize.width();
    height = identify->outputSize.height();
/*
    debugExtractedRAWData(rawData);
*/
    if (cancel)
    {
        return PROCESS_CANCELED;
    }

    return PROCESS_CONTINUE;
}

} // namespace Digikam
