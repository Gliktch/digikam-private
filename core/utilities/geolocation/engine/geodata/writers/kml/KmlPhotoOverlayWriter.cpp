/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "KmlPhotoOverlayWriter.h"

// Local includes

#include "GeoDataPhotoOverlay.h"
#include "GeoDataTypes.h"
#include "GeoDataImagePyramid.h"
#include "GeoDataPoint.h"
#include "GeoDataViewVolume.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLookAt(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataPhotoOverlayType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlPhotoOverlayWriter);

KmlPhotoOverlayWriter::KmlPhotoOverlayWriter() : KmlOverlayTagWriter(QString::fromUtf8(kml::kmlTag_PhotoOverlay))
{
    // nothing to do
}

bool KmlPhotoOverlayWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    KmlOverlayTagWriter::writeMid(node, writer);

    const GeoDataPhotoOverlay* photo_overlay =
        static_cast<const GeoDataPhotoOverlay*>(node);

    // rotation
    QString const rotation = QString::number(photo_overlay->rotation(), 'f', 3);
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_rotation), rotation, QString::fromUtf8("0.000"));

    // ViewVolume
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_ViewVolume));
    writer.writeOptionalElement<qreal>(QString::fromUtf8(kml::kmlTag_leftFov), photo_overlay->viewVolume().leftFov(), 0);
    writer.writeOptionalElement<qreal>(QString::fromUtf8(kml::kmlTag_rightFov), photo_overlay->viewVolume().rightFov(), 0);
    writer.writeOptionalElement<qreal>(QString::fromUtf8(kml::kmlTag_bottomFov), photo_overlay->viewVolume().bottomFov(), 0);
    writer.writeOptionalElement<qreal>(QString::fromUtf8(kml::kmlTag_topFov), photo_overlay->viewVolume().topFov(), 0);
    writer.writeOptionalElement<qreal>(QString::fromUtf8(kml::kmlTag_near), photo_overlay->viewVolume().near(), 0);
    writer.writeEndElement();

    // ImagePyramid
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_ImagePyramid));
    writer.writeOptionalElement<int>(QString::fromUtf8(kml::kmlTag_tileSize), photo_overlay->imagePyramid().tileSize(), 256);
    writer.writeOptionalElement<int>(QString::fromUtf8(kml::kmlTag_maxWidth), photo_overlay->imagePyramid().maxWidth());
    writer.writeOptionalElement<int>(QString::fromUtf8(kml::kmlTag_maxHeight), photo_overlay->imagePyramid().maxHeight());

    switch (photo_overlay->imagePyramid().gridOrigin())
    {
        case GeoDataImagePyramid::LowerLeft:
            writer.writeElement(QString::fromUtf8(kml::kmlTag_gridOrigin), QString::fromUtf8("lowerLeft"));
            break;

        case GeoDataImagePyramid::UpperLeft:
            writer.writeElement(QString::fromUtf8(kml::kmlTag_gridOrigin), QString::fromUtf8("upperLeft"));
            break;
    }

    writer.writeEndElement();

    // Point
    writeElement(&photo_overlay->point(), writer);

    // shape
    switch (photo_overlay->shape())
    {
        case GeoDataPhotoOverlay::Rectangle:
            break;

        case GeoDataPhotoOverlay::Cylinder:
            writer.writeElement(QString::fromUtf8(kml::kmlTag_shape), QString::fromUtf8("cylinder"));
            break;

        case GeoDataPhotoOverlay::Sphere:
            writer.writeElement(QString::fromUtf8(kml::kmlTag_shape), QString::fromUtf8("sphere"));
            break;
    }

    return true;
}

} // namespace Marble
