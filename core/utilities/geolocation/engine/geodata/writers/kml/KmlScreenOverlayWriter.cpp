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

#include "KmlScreenOverlayWriter.h"

// Local includes

#include "GeoDataScreenOverlay.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "GeoTagWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{
static GeoTagWriterRegistrar s_writerLookAt(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataScreenOverlayType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlScreenOverlayWriter);

KmlScreenOverlayWriter::KmlScreenOverlayWriter() : KmlOverlayTagWriter(QString::fromUtf8(kml::kmlTag_ScreenOverlay))
{
    // nothing to do
}


bool KmlScreenOverlayWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    KmlOverlayTagWriter::writeMid(node, writer);

    const GeoDataScreenOverlay* screenOverlay = static_cast<const GeoDataScreenOverlay*>(node);
    writeVec2(QString::fromUtf8(kml::kmlTag_overlayXY), screenOverlay->overlayXY(), writer);
    writeVec2(QString::fromUtf8(kml::kmlTag_screenXY), screenOverlay->screenXY(), writer);
    writeVec2(QString::fromUtf8(kml::kmlTag_rotationXY), screenOverlay->rotationXY(), writer);
    writeVec2(QString::fromUtf8(kml::kmlTag_size), screenOverlay->size(), writer);
    QString const rotation = QString::number(screenOverlay->rotation());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_rotation), rotation, QString::fromUtf8("0"));
    return true;
}

void KmlScreenOverlayWriter::writeVec2(const QString& element, const GeoDataVec2& vec2, GeoWriter& writer)
{
    writer.writeStartElement(element);
    writer.writeAttribute(QLatin1String("x"), QString::number(vec2.x()));
    writer.writeAttribute(QLatin1String("xunits"), unitToString(vec2.xunit()));
    writer.writeAttribute(QLatin1String("y"), QString::number(vec2.y()));
    writer.writeAttribute(QLatin1String("yunits"), unitToString(vec2.yunit()));
    writer.writeEndElement();
}

QString KmlScreenOverlayWriter::unitToString(GeoDataVec2::Unit unit)
{
    switch (unit)
    {
        case GeoDataVec2::Fraction:
            return QString::fromUtf8("fraction");

        case GeoDataVec2::Pixels:
            return QString::fromUtf8("pixels");

        case GeoDataVec2::InsetPixels:
            return QString::fromUtf8("insetPixels");
    }

    Q_ASSERT(false);
    return QString::fromUtf8("fraction");
}

} // namespace Marble
