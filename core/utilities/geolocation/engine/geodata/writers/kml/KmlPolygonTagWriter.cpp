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

#include "KmlPolygonTagWriter.h"

// Local includes

#include "GeoDataPolygon.h"
#include "GeoDataLinearRing.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLookAt(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataPolygonType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlPolygonTagWriter);

bool KmlPolygonTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataPolygon* polygon = static_cast<const GeoDataPolygon*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Polygon));
    KmlObjectTagWriter::writeIdentifiers(writer, polygon);
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_extrude), QString::number(polygon->extrude()), QString::fromUtf8("0"));

    writer.writeStartElement(QString::fromUtf8("outerBoundaryIs"));
    writeElement(&polygon->outerBoundary(), writer);
    writer.writeEndElement();

    const QVector<GeoDataLinearRing>& linearRings = polygon->innerBoundaries();

    if (linearRings.size() > 0)
    {
        writer.writeStartElement(QString::fromUtf8("innerBoundaryIs"));

        for (int i = 0; i < linearRings.size(); ++i)
        {
            const GeoDataLinearRing& ring = linearRings[i];
            writeElement(&ring, writer);
        }

        writer.writeEndElement();
    }

    writer.writeEndElement();

    return true;

}

} // namespace Marble
