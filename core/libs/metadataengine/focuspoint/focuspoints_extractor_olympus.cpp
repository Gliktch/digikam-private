/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 17/09/2025
 * Description : Extraction of focus points by exiftool data - Olympus devices
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 * SPDX-FileCopyrightText: 2025      by Maik Qualmann <metzpinguin at gmail dot com>
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

namespace OlympusInternal
{

FocusPoint create_af_point(float imageWidth,
                           float imageHeight,
                           float af_x_position,
                           float af_y_position)
{
    float maxSize       = imageWidth + imageHeight;
    float af_point_size = maxSize * 2.0F / 100.0F;

    return FocusPoint(af_x_position,
                      af_y_position,
                      af_point_size / imageWidth,
                      af_point_size / imageHeight,
                      FocusPoint::TypePoint::SelectedInFocus);
}

} // namespace OlympusInternal

FocusPointsExtractor::ListAFPoints FocusPointsExtractor::getAFPoints_olympus() const
{
    QString TagNameRoot  = QLatin1String("MakerNotes.Olympus.Camera");

    // Get size image

    QVariant imageWidth  = findValueFirstMatch(QStringList()
                                               << QLatin1String("File.File.Image.ImageWidth")
                                               << QLatin1String("EXIF.IFD0.Image.ImageWidth")
    );

    QVariant imageHeight = findValueFirstMatch(QStringList()
                                               << QLatin1String("File.File.Image.ImageHeight")
                                               << QLatin1String("EXIF.IFD0.Image.ImageHeight")
    );

    if (imageWidth.isNull() || imageHeight.isNull())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid Olympus/OMDS image sizes.";

        return getAFPoints_exif();
    }

    if (imageWidth.toInt() < imageHeight.toInt())
    {
        imageWidth.swap(imageHeight);
    }

    setOriginalSize(QSize(imageWidth.toInt(), imageHeight.toInt()));

    // Get af point

    QString af_string = findValue(TagNameRoot, QLatin1String("AFPointSelected")).toString();

    if (af_string.isEmpty())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid positions from Olympus/OMDS makernotes.";

        return getAFPoints_exif();
    }

    af_string.replace(QLatin1Char(' '), QLatin1Char(','));
    af_string.remove(QLatin1Char('('));
    af_string.remove(QLatin1Char(')'));
    af_string.remove(QLatin1Char('%'));

    QStringList af_position = af_string.split(QLatin1String(","));

    if (af_position.size() != 4)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid positions from Olympus/OMDS makernotes.";

        return getAFPoints_exif();
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Olympus/OMDS Makernotes Focus Location:" << af_position;

    float af_x_position = af_position[0].toFloat() / 100.0F;
    float af_y_position = af_position[1].toFloat() / 100.0F;

    // Add point

    ListAFPoints points;
    FocusPoint afpoint  = OlympusInternal::create_af_point(
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
