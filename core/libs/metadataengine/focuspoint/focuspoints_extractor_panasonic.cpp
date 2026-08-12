/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Extraction of focus points by exiftool data - Panasonic devices
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

// Internal function to create af point from meta data

namespace PanasonicInternal
{

FocusPoint create_af_point(float imageWidth,
                           float imageHeight,
                           float af_x_position,
                           float af_y_position)
{
    float maxSize       = imageWidth + imageHeight;
    float af_point_size = maxSize * 5.5F / 100.0F;

    return FocusPoint(af_x_position,
                      af_y_position,
                      af_point_size / imageWidth,
                      af_point_size / imageHeight,
                      FocusPoint::TypePoint::SelectedInFocus);
}

} // namespace PanasonicInternal

FocusPointsExtractor::ListAFPoints FocusPointsExtractor::getAFPoints_panasonic() const
{
    QString TagNameRoot  = QLatin1String("MakerNotes.Panasonic.Camera");

    // Get size image

    QVariant imageWidth  = findValue(QLatin1String("File.File.Image.ImageWidth"));
    QVariant imageHeight = findValue(QLatin1String("File.File.Image.ImageHeight"));

    if (imageWidth.isNull() || imageHeight.isNull())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid Panasonic image sizes.";

        return getAFPoints_exif();
    }

    if (imageWidth.toInt() < imageHeight.toInt())
    {
        imageWidth.swap(imageHeight);
    }

    setOriginalSize(QSize(imageWidth.toInt(), imageHeight.toInt()));

    // Get af point

    QStringList af_position = findValue(TagNameRoot, QLatin1String("AFPointPosition")).toString().split(QLatin1String(","));

    if (af_position.size() == 2)
    {
        af_position = af_position.at(0).split(QLatin1String(" "));
    }
    else
    {
        af_position = findValue(TagNameRoot, QLatin1String("AFPointPosition")).toString().split(QLatin1String(" "));
    }

    if (af_position.isEmpty() || (af_position.size() == 1))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid positions from Panasonic makernotes.";

        return getAFPoints_exif();
    }


    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Panasonic Makernotes Focus Location:" << af_position;

    float af_x_position = af_position[0].toFloat();
    float af_y_position = af_position[1].toFloat();

    // Add point

    ListAFPoints points;
    FocusPoint afpoint  = PanasonicInternal::create_af_point(
                                                             imageWidth.toFloat(),
                                                             imageHeight.toFloat(),
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
