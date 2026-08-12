/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG - Digital Negative creation.
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

int DNGWriter::Private::createNegative(AutoPtr<dng_negative>& negative,
                                       DRawInfo* const identify)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: DNG Negative structure creation";

    negative->SetDefaultScale(dng_urational(outputWidth,  activeWidth),
                              dng_urational(outputHeight, activeHeight));

    if (bayerPattern != Private::LinearRaw)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Bayer Pattern Linear Raw";
        negative->SetDefaultCropOrigin(8, 8);
        negative->SetDefaultCropSize(activeWidth - 16, activeHeight - 16);
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Bayer Pattern Type:" << bayerPattern;
        negative->SetDefaultCropOrigin(0, 0);
        negative->SetDefaultCropSize(activeWidth, activeHeight);
    }

    negative->SetActiveArea(activeArea);
    negative->SetModelName(identify->model.toLatin1().constData());
    negative->SetLocalName(QString::fromUtf8("%1 %2").arg(identify->make, identify->model).toLatin1().constData());
    negative->SetOriginalRawFileName(inputInfo.fileName().toLatin1().constData());
    negative->SetColorChannels(identify->rawColors);

    ColorKeyCode colorCodes[4] =
    {
        colorKeyMaxEnum,
        colorKeyMaxEnum,
        colorKeyMaxEnum,
        colorKeyMaxEnum
    };

    for (int i = 0 ; i < qMax(4, identify->colorKeys.length()) ; ++i)
    {
        if      (identify->colorKeys[i] == QLatin1Char('R'))
        {
            colorCodes[i] = colorKeyRed;
        }
        else if (identify->colorKeys[i] == QLatin1Char('G'))
        {
            colorCodes[i] = colorKeyGreen;
        }
        else if (identify->colorKeys[i] == QLatin1Char('B'))
        {
            colorCodes[i] = colorKeyBlue;
        }
        else if (identify->colorKeys[i] == QLatin1Char('C'))
        {
            colorCodes[i] = colorKeyCyan;
        }
        else if (identify->colorKeys[i] == QLatin1Char('M'))
        {
            colorCodes[i] = colorKeyMagenta;
        }
        else if (identify->colorKeys[i] == QLatin1Char('Y'))
        {
            colorCodes[i] = colorKeyYellow;
        }
    }

    negative->SetColorKeys(colorCodes[0], colorCodes[1], colorCodes[2], colorCodes[3]);

    switch (bayerPattern)
    {
        case Private::Standard:
        {
            // Standard bayer mosaicing. All work fine there.
            // Bayer CCD mask: https://en.wikipedia.org/wiki/Bayer_filter

            negative->SetBayerMosaic(filter);
            break;
        }

        case Private::Fuji:
        {
            // TODO: Fuji is special case. Need to setup different bayer rules here.
            // It do not work in all settings. Need indeep investiguations.
            // Fuji superCCD: https://en.wikipedia.org/wiki/Super_CCD

            negative->SetFujiMosaic(filter);
            break;
        }

        case Private::Fuji6x6:
        {
            // TODO: Fuji is special case. Need to setup different bayer rules here.
            // It do not work in all settings. Need indeep investiguations.

            negative->SetFujiMosaic6x6(filter);
            break;
        }

        case Private::FourColor:
        {
            negative->SetQuadMosaic(filter);
            break;
        }

        default:
        {
            break;
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter:" << dngBayerPatternToString(bayerPattern);

    negative->SetWhiteLevel(identify->whitePoint, 0);
    negative->SetWhiteLevel(identify->whitePoint, 1);
    negative->SetWhiteLevel(identify->whitePoint, 2);
    negative->SetWhiteLevel(identify->whitePoint, 3);

    const dng_mosaic_info* const mosaicinfo = negative->GetMosaicInfo();

    if ((mosaicinfo                  != nullptr) &&
        (mosaicinfo->fCFAPatternSize == dng_point(2, 2)))
    {
        negative->SetQuadBlacks(identify->blackPoint + identify->blackPointCh[0],
                                identify->blackPoint + identify->blackPointCh[1],
                                identify->blackPoint + identify->blackPointCh[2],
                                identify->blackPoint + identify->blackPointCh[3]);
    }
    else
    {
        negative->SetBlackLevel(identify->blackPoint, 0);
    }

    negative->SetBaselineExposure(0.0);
    negative->SetBaselineNoise(1.0);
    negative->SetBaselineSharpness(1.0);

    dng_orientation orientation;

    switch (identify->orientation)
    {
        case DRawInfo::ORIENTATION_180:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Mosaic orientation: rotate 180";
            orientation = dng_orientation::Rotate180();
            break;
        }

        case DRawInfo::ORIENTATION_Mirror90CCW:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Mosaic orientation: mirror 90CCW";
            orientation = dng_orientation::Mirror90CCW();
            break;
        }

        case DRawInfo::ORIENTATION_90CCW:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Mosaic orientation: rotate 90CCW";
            orientation = dng_orientation::Rotate90CCW();
            break;
        }

        case DRawInfo::ORIENTATION_90CW:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Mosaic orientation: rotate 90CW";
            orientation = dng_orientation::Rotate90CW();
            break;
        }

        default:   // ORIENTATION_NONE
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Mosaic orientation: no rotation";
            orientation = dng_orientation::Normal();
            break;
        }
    }

    negative->SetBaseOrientation(orientation);
    negative->SetAntiAliasStrength(dng_urational(100, 100));
    negative->SetLinearResponseLimit(1.0);
    negative->SetShadowScale( dng_urational(1, 1) );
    negative->SetAnalogBalance(dng_vector_3(1.0, 1.0, 1.0));

    // -------------------------------------------------------------------------------

    AutoPtr<dng_camera_profile> prof(new dng_camera_profile);
    prof->SetName(QString::fromUtf8("%1 %2").arg(identify->make, identify->model).toLatin1().constData());

    // Set Camera->XYZ Color matrix as profile.

    dng_matrix matrix;

    switch (identify->rawColors)
    {
        case 3:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Mosaic Raw color components: 3";

            dng_matrix_3by3 camXYZ;
            camXYZ[0][0] = identify->cameraXYZMatrix[0][0];
            camXYZ[0][1] = identify->cameraXYZMatrix[0][1];
            camXYZ[0][2] = identify->cameraXYZMatrix[0][2];
            camXYZ[1][0] = identify->cameraXYZMatrix[1][0];
            camXYZ[1][1] = identify->cameraXYZMatrix[1][1];
            camXYZ[1][2] = identify->cameraXYZMatrix[1][2];
            camXYZ[2][0] = identify->cameraXYZMatrix[2][0];
            camXYZ[2][1] = identify->cameraXYZMatrix[2][1];
            camXYZ[2][2] = identify->cameraXYZMatrix[2][2];

            if (camXYZ.MaxEntry() == 0.0)
            {
                qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: camera XYZ Matrix is null : camera not supported";
                return FILE_NOT_SUPPORTED;
            }

            matrix = camXYZ;

            break;
        }

        case 4:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Mosaic Raw color components: 3";

            dng_matrix_4by3 camXYZ;
            camXYZ[0][0] = identify->cameraXYZMatrix[0][0];
            camXYZ[0][1] = identify->cameraXYZMatrix[0][1];
            camXYZ[0][2] = identify->cameraXYZMatrix[0][2];
            camXYZ[1][0] = identify->cameraXYZMatrix[1][0];
            camXYZ[1][1] = identify->cameraXYZMatrix[1][1];
            camXYZ[1][2] = identify->cameraXYZMatrix[1][2];
            camXYZ[2][0] = identify->cameraXYZMatrix[2][0];
            camXYZ[2][1] = identify->cameraXYZMatrix[2][1];
            camXYZ[2][2] = identify->cameraXYZMatrix[2][2];
            camXYZ[3][0] = identify->cameraXYZMatrix[3][0];
            camXYZ[3][1] = identify->cameraXYZMatrix[3][1];
            camXYZ[3][2] = identify->cameraXYZMatrix[3][2];

            if (camXYZ.MaxEntry() == 0.0)
            {
                qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: camera XYZ Matrix is null : camera not supported";
                return FILE_NOT_SUPPORTED;
            }

            matrix = camXYZ;

            break;
        }

        default:
        {
            qCCritical(DIGIKAM_GENERAL_LOG) << "DNGWriter: Mosaic Raw color components not handled:" << identify->rawColors;
        }
    }

    prof->SetColorMatrix1((dng_matrix) matrix);
    prof->SetCalibrationIlluminant1(lsD65);
    negative->AddProfile(prof);

    dng_vector camNeutral(identify->rawColors);

    for (int i = 0 ; i < identify->rawColors ; ++i)
    {
        camNeutral[i] = 1.0 / identify->cameraMult[i];
    }

    negative->SetCameraNeutral(camNeutral);

    if (cancel)
    {
        return PROCESS_CANCELED;
    }

    return PROCESS_CONTINUE;
}

} // namespace Digikam
