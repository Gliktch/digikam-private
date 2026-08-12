/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Extraction of focus points by exiftool data - Sony devices
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "focuspoints_extractor.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

const float RATIO_POINT_IMAGE = 120; // this is a guess

// Internal function to create af point from meta data

namespace SonyInternal
{

FocusPoint create_af_point1(float imageWidth,
                            float imageHeight,
                            float afPointWidth,
                            float afPointHeight,
                            float af_x_position,
                            float af_y_position)
{
    return FocusPoint(af_x_position / imageWidth,
                      af_y_position / imageHeight,
                      afPointWidth,
                      afPointHeight,
                      FocusPoint::TypePoint::SelectedInFocus);
}

FocusPoint create_af_point2(float imageWidth,
                            float imageHeight,
                            float afPointWidth,
                            float afPointHeight,
                            float af_x_position,
                            float af_y_position)
{
    return FocusPoint(af_x_position / imageWidth,
                      af_y_position / imageHeight,
                      afPointWidth  / imageWidth,
                      afPointHeight / imageHeight,
                      FocusPoint::TypePoint::SelectedInFocus);
}

} // namespace SonyInternal

// Main function to extract af point
FocusPointsExtractor::ListAFPoints FocusPointsExtractor::getAFPoints_sony() const
{
    QString TagNameRoot      = QLatin1String("MakerNotes.Sony.Camera");
    QStringList af_info      = findValue(TagNameRoot, QLatin1String("FocusLocation")).toString().split(QLatin1String(" "));
    QStringList af_frameSize = findValue(TagNameRoot, QLatin1String("FocusFrameSize")).toString().split(QLatin1String("x"));

    if  (af_info.size() < 5)
    {
        if (af_info.size() == 4)
        {
            // Get size image

            float afImageWidth  = af_info[0].toFloat();
            float afImageHeight = af_info[1].toFloat();

            // Get size of af points

            // For cameras without a focus frame size we initially
            // set a default value. This must be validated later.

            float afPointWidth  = 300.0F;
            float afPointHeight = 300.0F;

            if (af_frameSize.size() == 2)
            {
                afPointWidth  = af_frameSize[0].toFloat();
                afPointHeight = af_frameSize[1].toFloat();
            }

            // Get coordinate of af points

            float af_x_position = af_info[2].toFloat();
            float af_y_position = af_info[3].toFloat();

             ListAFPoints points;
             FocusPoint afpoint = SonyInternal::create_af_point2(
                                                                 afImageWidth,
                                                                 afImageHeight,
                                                                 afPointWidth,
                                                                 afPointHeight,
                                                                 af_x_position,
                                                                 af_y_position
                                                                );

            if (afpoint.getRect().isValid())
            {
                points << afpoint;
            }

            return points;
        }
        else
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Unsupported Sony Camera.";

            return getAFPoints_exif();
        }
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Sony Makernotes Focus Location:" << af_info;

    // Get size image

    float afImageWidth  = af_info[0].toFloat();
    float afImageHeight = af_info[1].toFloat();

    // Get size of af points

    float afPointWidth  = afImageWidth  * RATIO_POINT_IMAGE;
    float afPointHeight = afImageHeight * RATIO_POINT_IMAGE;

    // Get coordinate of af points

    float af_x_position = af_info[3].toFloat();
    float af_y_position = af_info[4].toFloat();

    ListAFPoints points;
    FocusPoint afpoint  = SonyInternal::create_af_point1(
                                                         afImageWidth,
                                                         afImageHeight,
                                                         afPointWidth,
                                                         afPointHeight,
                                                         af_x_position,
                                                         af_y_position
                                                        );

    if (afpoint.getRect().isValid())
    {
        points << afpoint;
    }

    return points;
}

} // namespace Digikam
