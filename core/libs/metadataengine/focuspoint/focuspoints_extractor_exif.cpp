/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Extraction of focus points by exiftool data - Exif metadata
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

namespace ExifInternal
{

FocusPoint create_af_point(float imageWidth,
                           float imageHeight,
                           float af_x_position,
                           float af_y_position,
                           float afPointWidth,
                           float afPointHeight,
                           MetaEngine::ImageOrientation orientation)
{
    QRect region;
    region.moveCenter(QPoint(af_x_position, af_y_position));
    region.setSize(QSize(afPointWidth, afPointHeight));
    QSize size = QSize(imageWidth, imageHeight);

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Exif Subject Area before rotation:" << region;

    if      (
             (orientation == MetaEngine::ORIENTATION_ROT_90)       ||
             (orientation == MetaEngine::ORIENTATION_ROT_90_HFLIP) ||
             (orientation == MetaEngine::ORIENTATION_ROT_90_VFLIP)
            )
    {
        region.moveTo(size.height() - region.y() - region.height(), region.x());
        region.setSize(region.size().transposed());
    }
    else if (orientation == MetaEngine::ORIENTATION_ROT_180)
    {
        region.moveTo(size.width()  - region.x() - region.width(),
                      size.height() - region.y() - region.height());

    }
    else if (orientation == MetaEngine::ORIENTATION_ROT_270)
    {
        region.moveTo(region.y(), size.width() - region.x() - region.width());
        region.setSize(region.size().transposed());
    }

    if      (
             (orientation == MetaEngine::ORIENTATION_HFLIP) ||
             (orientation == MetaEngine::ORIENTATION_ROT_90_HFLIP)
            )
    {
        region.moveTo(size.width() - region.x() - region.width(), region.y());
    }
    else if (
             (orientation == MetaEngine::ORIENTATION_VFLIP) ||
             (orientation == MetaEngine::ORIENTATION_ROT_90_VFLIP)
            )
    {
        region.moveTo(region.x(), size.height() - region.y() - region.height());
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Exif Subject Area after rotation:" << region;

    FocusPoint fp(region.center().x() / imageWidth,
                  region.center().y() / imageHeight,
                  region.width()      / imageWidth,
                  region.height()     / imageHeight,
                  FocusPoint::TypePoint::SelectedInFocus);

    return fp;
}

} // namespace ExifInternal

// Main function to extract af point
FocusPointsExtractor::ListAFPoints FocusPointsExtractor::getAFPoints_exif() const
{
    // NOTE: See documentation of this tag contents: https://www.awaresystems.be/imaging/tiff/tifftags/privateifd/exif/subjectarea.html

    QStringList af_info = findValue(QLatin1String("EXIF.ExifIFD.Camera.SubjectArea")).toString()
                                                                                     .split(QLatin1String(" "));

    float af_x_position = 0;
    float af_y_position = 0;
    float afPointWidth  = 0;
    float afPointHeight = 0;

    switch (af_info.size())
    {
        case 4:
        {
            // Get center coordinates of AF point.

            af_x_position = af_info[0].toFloat();
            af_y_position = af_info[1].toFloat();

            // Get size of AF area.

            afPointWidth  = af_info[2].toFloat();
            afPointHeight = af_info[3].toFloat();

            break;
        }

        case 3:
        {
            // Get center coordinates of AF point.

            af_x_position = af_info[0].toFloat();
            af_y_position = af_info[1].toFloat();

            // Get size of AF area (typically a circle transformed as rectangle).

            afPointWidth  = af_info[3].toFloat();
            afPointHeight = af_info[3].toFloat();

            break;
        }

        case 2:
        {
            // Get center coordinates of AF point.

            af_x_position = af_info[0].toFloat();
            af_y_position = af_info[1].toFloat();

            // Get size of AF area (typically a point transformed an arbitrary square of size 120 pixels).

            afPointWidth  = 120.0F;
            afPointHeight = 120.0F;

            break;
        }

        default:    // Other sizes are not valid
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Invalid Exif Subject Area.";

            // Fail-back to XMP metadata if exists.

            return getAFPoints_xmp();
        }
    }

    QSizeF fs = QSizeF(originalSize());

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Exif Subject Area:" << af_info;

    ListAFPoints points;
    FocusPoint afpoint = ExifInternal::create_af_point(
                                                       fs.width(),
                                                       fs.height(),
                                                       af_x_position,
                                                       af_y_position,
                                                       afPointWidth,
                                                       afPointHeight,
                                                       orientation()
                                                      );

    if (afpoint.getRect().isValid())
    {
        points << afpoint;
    }

    return points;
}

} // namespace Digikam
