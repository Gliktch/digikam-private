/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Extraction of focus points by exiftool data - Nikon devices
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

namespace NikonInternal
{

FocusPoint create_af_point(float imageWidth,
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

} // namespace NikonInternal

// Main function to extract af point
FocusPointsExtractor::ListAFPoints FocusPointsExtractor::getAFPoints_nikon() const
{
    QString TagNameRoot = QLatin1String("MakerNotes.Nikon.Camera");

    // Filter model

    if (!model().contains(QLatin1String("NIKON Z")))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: unsupported Nikon Camera.";

        return getAFPoints_exif();
    }

    // Get image size

    QVariant imageWidth, imageHeight;

    imageWidth = findValueFirstMatch(
                                     QStringList()
                                     << QLatin1String("MakerNotes.Nikon.Camera.AFImageWidth")
                                     << QLatin1String("EXIF.ExifIFD.Image.ExifImageWidth")
                                     << QLatin1String("EXIF.SubIFD1.Image.ImageWidth")
                                     << QLatin1String("File.File.Image.ImageWidth")
                                    );

    imageHeight = findValueFirstMatch(
                                      QStringList()
                                      << QLatin1String("MakerNotes.Nikon.Camera.AFImageHeight")
                                      << QLatin1String("EXIF.ExifIFD.Image.ExifImageHeight")
                                      << QLatin1String("EXIF.SubIFD1.Image.ImageHeight")
                                      << QLatin1String("File.File.Image.ImageHeight")
                                     );

    if (imageWidth.isNull() || imageHeight.isNull())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid Nikon image sizes.";

        return getAFPoints_exif();
    }

    if (imageWidth.toInt() < imageHeight.toInt())
    {
        imageWidth.swap(imageHeight);
    }

    setOriginalSize(QSize(imageWidth.toInt(), imageHeight.toInt()));

    const QStringList modelList({
                                 QLatin1String("NIKON Z 5"),
                                 QLatin1String("NIKON Z5_2")
                                });

    const QList<float> posList({
                                11.5F, 11.5F, -6.5F, -6.5F,   // Z5
                                15.5F,  9.3F,  4.8F,  7.0F    // Z5 II
                               });

    int camIdx = modelList.indexOf(model());

    if (camIdx != -1)
    {
        QStringList afPointUsed = findValueFirstMatch(
                                                      QStringList()
                                                      << QLatin1String("MakerNotes.Nikon.Camera.AFPointsUsed"),
                                                      true
                                                     ).toStringList();

        if (!afPointUsed.isEmpty() && !afPointUsed.contains(QLatin1String("(none)")))
        {
            camIdx *= 4;
            ListAFPoints points;

            float xOffset = imageWidth.toInt()  / posList.at(camIdx + 0);
            float yOffset = imageHeight.toInt() / posList.at(camIdx + 1);

            float xShift  = imageWidth.toInt()  / posList.at(camIdx + 2);
            float yShift  = imageHeight.toInt() / posList.at(camIdx + 3);

            for (const QString& np : EXIV2_AS_CONST(afPointUsed))
            {
                if (np.size() < 2)
                {
                    continue;
                }

                float xPosition = (np.mid(1).toInt() - 1)    * xOffset - xShift;
                float yPosition = (np[0].unicode()   - 0x41) * yOffset - yShift;

                FocusPoint afpoint = NikonInternal::create_af_point(
                                                                    imageWidth.toFloat(),
                                                                    imageHeight.toFloat(),
                                                                    xOffset - 30.0F,
                                                                    yOffset - 30.0F,
                                                                    xPosition,
                                                                    yPosition
                                                                   );

                if (afpoint.getRect().isValid())
                {
                    points << afpoint;
                }
            }

            return points;
        }
    }

    // Get size of point

    QVariant afPointWidth  = findValue(TagNameRoot, QLatin1String("AFAreaWidth"));
    QVariant afPointHeight = findValue(TagNameRoot, QLatin1String("AFAreaHeight"));

    if ((afPointWidth.isNull()) || (afPointHeight.isNull()))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid sizes from Nikon makernotes.";

        return getAFPoints_exif();
    }

    // Get coordinate of af points

    QVariant af_x_position = findValue(TagNameRoot, QLatin1String("AFAreaXPosition"));
    QVariant af_y_position = findValue(TagNameRoot, QLatin1String("AFAreaYPosition"));

    if (af_x_position.isNull() || af_y_position.isNull())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: invalid positions from Nikon makernotes.";

        return getAFPoints_exif();
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: Nikon Makernotes Focus Location:" << af_x_position
                                                                                                << af_y_position
                                                                                                << afPointWidth
                                                                                                << afPointHeight;

    ListAFPoints points;
    FocusPoint afpoint = NikonInternal::create_af_point(
                                                        imageWidth.toFloat(),
                                                        imageHeight.toFloat(),
                                                        afPointWidth.toFloat(),
                                                        afPointHeight.toFloat(),
                                                        af_x_position.toFloat(),
                                                        af_y_position.toFloat()
                                                       );

    if (afpoint.getRect().isValid())
    {
        points << afpoint;
    }

    return points;
}

} // namespace Digikam
