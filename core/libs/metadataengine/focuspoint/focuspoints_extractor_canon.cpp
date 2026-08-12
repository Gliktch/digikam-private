/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Extraction of focus points by exiftool data - Canon devices
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

namespace CanonInternal
{

void set_point_position(FocusPoint& point,
                        float imageWidth,
                        float imageHeight,
                        float af_x_position,
                        float af_y_position,
                        int yDirection)
{
    point.setCenterPosition(0.5 + af_x_position              / imageWidth,
                            0.5 + af_y_position * yDirection / imageHeight);
}

void set_point_size(FocusPoint& point,
                    float imageWidth,
                    float imageHeight,
                    float afPointWidth,
                    float afPointHeight)
{
    point.setSize(afPointWidth / imageWidth, afPointHeight / imageHeight);
}

void set_point_type(FocusPoint& point,
                    const QStringList& af_selected,
                    const QStringList& af_infocus,
                    int index)
{
    point.setType(FocusPoint::TypePoint::Inactive);

    if (af_infocus.isEmpty() && af_selected.contains(QString::number(index)))
    {
        point.setType(FocusPoint::TypePoint::Selected);
    }

    if (af_infocus.contains(QString::number(index)))
    {
        point.setType(FocusPoint::TypePoint::SelectedInFocus);
    }
}

FocusPoint create_af_point(float imageWidth,
                           float imageHeight,
                           float afPointWidth,
                           float afPointHeight,
                           float af_x_position,
                           float af_y_position,
                           const QStringList& af_selected,
                           const QStringList& af_infocus,
                           int   yDirection,
                           int   index)
{
    FocusPoint point;

    set_point_position(point,
                       imageWidth,
                       imageHeight,
                       af_x_position,
                       af_y_position,
                       yDirection);

    set_point_size(point,
                   imageWidth,
                   imageHeight,
                   afPointWidth,
                   afPointHeight);

    set_point_type(point,
                   af_selected,
                   af_infocus,
                   index);

    return point;
}

} // namespace CanonInternal

// Main function to extract af point
FocusPointsExtractor::ListAFPoints FocusPointsExtractor::getAFPoints_canon() const
{
    QString TagNameRoot = QLatin1String("MakerNotes.Canon.Camera");

    // Get size image

    QVariant imageWidth, imageHeight;

    if (model() == QLatin1String("CANON EOS 5D"))
    {
        imageWidth  = findValueFirstMatch(QStringList()
                                          << QLatin1String("MakerNotes.Canon.Image.CanonImageWidth")
                                          << QLatin1String("EXIF.ExifIFD.Image.ExifImageWidth")
        );

        imageHeight = findValueFirstMatch(QStringList()
                                          << QLatin1String("MakerNotes.Canon.Image.CanonImageHeight")
                                          << QLatin1String("EXIF.ExifIFD.Image.ExifImageHeight")
        );
    }
    else
    {
        imageWidth = findValueFirstMatch(QStringList()
                                         << QLatin1String("MakerNotes.Canon.Camera.AFImageWidth")
                                         << QLatin1String("EXIF.ExifIFD.Image.ExifImageWidth")
        );

        imageHeight = findValueFirstMatch(QStringList()
                                          << QLatin1String("MakerNotes.Canon.Camera.AFImageHeight")
                                          << QLatin1String("EXIF.ExifIFD.Image.ExifImageHeight")
        );
    }

    if (imageWidth.isNull() || imageHeight.isNull())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid Canon Camera image sizes.";

        return getAFPoints_exif();
    }

    if (imageWidth.toInt() < imageHeight.toInt())
    {
        imageWidth.swap(imageHeight);
    }

    setOriginalSize(QSize(imageWidth.toInt(), imageHeight.toInt()));

    // Get size of af points

    QVariant afPointWidth      = findValue(TagNameRoot, QLatin1String("AFAreaWidth"));
    QVariant afPointHeight     = findValue(TagNameRoot, QLatin1String("AFAreaHeight"));
    QStringList afPointWidths  = findValue(TagNameRoot, QLatin1String("AFAreaWidths"),  true).toStringList();
    QStringList afPointHeights = findValue(TagNameRoot, QLatin1String("AFAreaHeights"), true).toStringList();

    if (
        (afPointWidth.isNull()   || afPointHeight.isNull()) &&
        (afPointWidths.isEmpty() || afPointHeights.isEmpty())
       )
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid sizes from Canon makernotes.";

        return getAFPoints_exif();
    }

    // Get coordinate of af points

    QStringList af_x_positions = findValue(TagNameRoot, QLatin1String("AFAreaXPositions"), true).toStringList();
    QStringList af_y_positions = findValue(TagNameRoot, QLatin1String("AFAreaYPositions"), true).toStringList();

    if (af_x_positions.isEmpty() || af_y_positions.isEmpty())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid positions from Canon makernotes.";

        return getAFPoints_exif();
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Canon makernotes focus Xs/Ys size:" << af_x_positions.size()
                                                                                                  << af_y_positions.size();

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Canon makernotes focus W/H value :" << afPointWidth
                                                                                                  << afPointHeight;

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Canon makernotes focus Ws/Hs size:" << afPointWidths.size()
                                                                                                  << afPointHeights.size();

    // Get type of af points

    QStringList af_selected = findValueFirstMatch(TagNameRoot, QStringList() << QLatin1String("AFPointsSelected")
                                                                             << QLatin1String("AFPointsInFocus"),
                                                  true).toStringList();
    QStringList af_infocus  = findValue(TagNameRoot, QLatin1String("AFPointsInFocus"), true).toStringList();

    // Check and remove possible (none) keyword

    if (af_infocus.size() == 1)
    {
        bool ok     = false;
        int afPoint = af_infocus.first().toInt(&ok);

        if ((afPoint == 0) && !ok)
        {
            af_infocus.clear();
        }
    }

    // If we have focus points in AFPointsInFocus,
    // remove them from AFPointsSelected

    for (const QString& key : EXIV2_AS_CONST(af_infocus))
    {
        af_selected.removeAll(key);
    }

    // Get direction

    QString cameraType = findValue(TagNameRoot, QLatin1String("CameraType")).toString().toUpper();
    int yDirection     = 1;

    if (
        (cameraType == QLatin1String("COMPACT"))   ||
        (cameraType == QLatin1String("EOS HIGH-END"))
       )
    {
        yDirection = -1;
    }

    ListAFPoints points;

    for (int i = 0 ; i < af_x_positions.count() ; ++i)
    {
        float afPointWidthUsed  = (afPointWidths.isEmpty()) ? afPointWidth.toFloat()
                                                            : afPointWidths[i].toFloat();

        float afPointHeightUsed = (afPointHeights.isEmpty()) ? afPointHeight.toFloat()
                                                             : afPointHeights[i].toFloat();

        FocusPoint point        = CanonInternal::create_af_point(
                                                                 imageWidth.toFloat(),
                                                                 imageHeight.toFloat(),
                                                                 afPointWidthUsed,
                                                                 afPointHeightUsed,
                                                                 af_x_positions[i].toFloat(),
                                                                 af_y_positions[i].toFloat(),
                                                                 af_selected,
                                                                 af_infocus,
                                                                 yDirection,
                                                                 i
                                                                );

        if (!point.getRect().isValid())
        {
            continue;
        }

        points.append(point);
    }

    return points;
}

} // namespace Digikam
