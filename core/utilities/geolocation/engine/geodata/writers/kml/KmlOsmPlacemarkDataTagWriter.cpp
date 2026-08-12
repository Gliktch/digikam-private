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

#include "KmlOsmPlacemarkDataTagWriter.h"

// Local includes

#include "GeoDataPlacemark.h"
#include "GeoDataSchemaData.h"
#include "GeoDataExtendedData.h"
#include "GeoDataFeature.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPolygon.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "OsmPlacemarkData.h"

namespace Marble
{

bool KmlOsmPlacemarkDataTagWriter::write(const GeoDataFeature* feature,
                                         GeoWriter& writer)
{
    const GeoDataPlacemark* placemark = geodata_cast<GeoDataPlacemark>(feature);

    if (!placemark)
    {
        return false;
    }

    const OsmPlacemarkData& osmData = placemark->osmData();

    if (osmData.isNull())
    {
        return true;
    }

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_ExtendedData));

    // We declare the "mx" namespace for the custom osmPlacemarkData XML schema
    writer.writeNamespace(QString::fromUtf8(kml::kmlTag_nameSpaceMx), QLatin1String("mx"));

    KmlOsmPlacemarkDataTagWriter::writeOsmData(placemark->geometry(), osmData, writer);

    writer.writeEndElement();
    return true;
}

bool KmlOsmPlacemarkDataTagWriter::writeOsmData(const GeoDataGeometry* geometry,
                                                const OsmPlacemarkData& osmData,
                                                GeoWriter& writer)
{
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_nameSpaceMx), QString::fromUtf8(kml::kmlTag_OsmPlacemarkData));

    // Writing the attributes
    writer.writeAttribute(QString::fromUtf8("id"), QString::number(osmData.id()));
    writer.writeOptionalAttribute(QString::fromUtf8("changeset"), osmData.changeset());
    writer.writeOptionalAttribute(QString::fromUtf8("timestamp"), osmData.timestamp());
    writer.writeOptionalAttribute(QString::fromUtf8("uid"), osmData.uid());
    writer.writeOptionalAttribute(QString::fromUtf8("user"), osmData.user());
    writer.writeOptionalAttribute(QString::fromUtf8("version"), osmData.version());
    writer.writeOptionalAttribute(QString::fromUtf8("visible"), osmData.isVisible());
    writer.writeOptionalAttribute(QString::fromUtf8("action"), osmData.action());

    // Writing the tags
    QHash<QString, QString>::const_iterator tagsIt = osmData.tagsBegin();
    QHash<QString, QString>::const_iterator tagsEnd = osmData.tagsEnd();

    for (; tagsIt != tagsEnd; ++tagsIt)
    {
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_nameSpaceMx), QString::fromUtf8("tag"));
        writer.writeAttribute(QString::fromUtf8("k"), tagsIt.key());
        writer.writeAttribute(QString::fromUtf8("v"), tagsIt.value());
        writer.writeEndElement();
    }

    if (geometry)
    {

        // Ways
        if (const GeoDataLineString* lineString = dynamic_cast<const GeoDataLineString*>(geometry))
        {
            int ndIndex = 0;

            // Writing the component nodes
            QVector<GeoDataCoordinates>::const_iterator nodeIt = lineString->begin();
            QVector<GeoDataCoordinates>::const_iterator nodeEnd = lineString->end();

            for (; nodeIt != nodeEnd; ++nodeIt)
            {
                const OsmPlacemarkData& nodeOsmData = osmData.nodeReference(*nodeIt);
                writer.writeStartElement(QString::fromUtf8(kml::kmlTag_nameSpaceMx), QString::fromUtf8("nd"));
                writer.writeAttribute(QString::fromUtf8("index"), QString::number(ndIndex++));
                writeOsmData(nullptr, nodeOsmData, writer);
                writer.writeEndElement();
            }
        }

        // Polygons
        else if (const GeoDataPolygon* polygon = geodata_cast<GeoDataPolygon>(geometry))
        {
            int memberIndex = -1;

            // Writing the outerBoundary osmData
            const GeoDataLinearRing& outerRing = polygon->outerBoundary();
            const OsmPlacemarkData& outerRingOsmData = osmData.memberReference(memberIndex);
            writer.writeStartElement(QString::fromUtf8(kml::kmlTag_nameSpaceMx), QString::fromUtf8(kml::kmlTag_member));
            writer.writeAttribute(QString::fromUtf8("index"), QString::number(memberIndex));
            writeOsmData(&outerRing, outerRingOsmData, writer);
            writer.writeEndElement();

            // Writing the innerBoundaries
            for (const GeoDataLinearRing& innerRing : polygon->innerBoundaries())
            {
                const OsmPlacemarkData& innerRingOsmData = osmData.memberReference(++memberIndex);
                writer.writeStartElement(QString::fromUtf8(kml::kmlTag_nameSpaceMx), QString::fromUtf8(kml::kmlTag_member));
                writer.writeAttribute(QString::fromUtf8("index"), QString::number(memberIndex));
                writeOsmData(&innerRing, innerRingOsmData, writer);
                writer.writeEndElement();
            }
        }
    }

    writer.writeEndElement();
    return true;
}

} // namespace Marble
