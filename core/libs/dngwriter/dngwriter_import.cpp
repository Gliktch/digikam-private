/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG - Raw data import.
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

int DNGWriter::Private::importRaw(DRawInfo* const identify,
                                  DRawInfo* const identifyMake)
{
    if (parent->inputFile().isEmpty())
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: No input file to convert. Aborted...";

        return PROCESS_FAILED;
    }

    inputInfo   = QFileInfo(parent->inputFile());
    dngFilePath = parent->outputFile();

    if (dngFilePath.isEmpty())
    {
        dngFilePath = QString(inputInfo.completeBaseName() + QLatin1String(".dng"));
    }

    outputInfo  = QFileInfo(dngFilePath);

    // -----------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Loading RAW data from " << inputInfo.fileName();

    QPointer<DRawDecoder> rawProcessor(new DRawDecoder);

    if (!rawProcessor->rawFileIdentify(*identifyMake, parent->inputFile()))
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Reading RAW file failed. Aborted...";

        return PROCESS_FAILED;
    }

    // TODO: need to get correct default crop size to avoid artifacts at the borders

    if ((identifyMake->orientation == 5) || (identifyMake->orientation == 6))
    {
        outputHeight = identifyMake->outputSize.width();
        outputWidth  = identifyMake->outputSize.height();
    }
    else
    {
        outputHeight = identifyMake->outputSize.height();
        outputWidth  = identifyMake->outputSize.width();
    }

    if (!rawProcessor->extractRAWData(parent->inputFile(), rawData, *identify, 0))
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Loading RAW data failed. Aborted...";

        return FILE_NOT_SUPPORTED;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Raw data loaded:" ;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Data Size:     " << rawData.size() << " bytes";
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Date:          " << identify->dateTime.toString(Qt::ISODate);
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Make:          " << identify->make;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Model:         " << identify->model;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- ImageSize:     " << identify->imageSize.width()  << "x" << identify->imageSize.height();
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- FullSize:      " << identify->fullSize.width()   << "x" << identify->fullSize.height();
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- OutputSize:    " << identify->outputSize.width() << "x" << identify->outputSize.height();
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Orientation:   " << identify->orientation;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Top margin:    " << identify->topMargin;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Left margin:   " << identify->leftMargin;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Filter:        " << identify->filterPattern;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Colors:        " << identify->rawColors;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- Black:         " << identify->blackPoint;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- White:         " << identify->whitePoint;
    qCDebug(DIGIKAM_GENERAL_LOG) << "--- CAM->XYZ:";

    for (int i = 0 ; i < 4 ; ++i)
    {
        qCDebug(DIGIKAM_GENERAL_LOG)
                 << "                   "
                 << QString().asprintf("%03.4f  %03.4f  %03.4f", identify->cameraXYZMatrix[i][0],
                                                                 identify->cameraXYZMatrix[i][1],
                                                                 identify->cameraXYZMatrix[i][2]);
    }

    if (cancel)
    {
        return PROCESS_CANCELED;
    }

    return PROCESS_CONTINUE;
}

} // namespace Digikam
