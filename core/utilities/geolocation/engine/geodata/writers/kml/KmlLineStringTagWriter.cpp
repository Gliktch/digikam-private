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

#include "KmlLineStringTagWriter.h"

// Local includes

#include "GeoDataLineString.h"
#include "GeoDataTypes.h"
#include "GeoDataCoordinates.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLookAt(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataLineStringType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlLineStringTagWriter);

bool KmlLineStringTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataLineString* lineString = static_cast<const GeoDataLineString*>(node);

    if (lineString->size() > 1)
    {
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_LineString));
        KmlObjectTagWriter::writeIdentifiers(writer, lineString);
        writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_extrude), QString::number(lineString->extrude()), QString::fromUtf8("0"));
        writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_tessellate), QString::number(lineString->tessellate()), QString::fromUtf8("0"));
        writer.writeStartElement(QString::fromUtf8("coordinates"));

        // Write altitude for *all* elements, if *any* element
        // has altitude information (!= 0.0)
        bool hasAltitude = false;

        for (int i = 0; i < lineString->size(); ++i)
        {
            if (lineString->at(i).altitude())
            {
                hasAltitude = true;
                break;
            }
        }

        for (int i = 0; i < lineString->size(); ++i)
        {
            GeoDataCoordinates coordinates = lineString->at(i);

            if (i > 0)
            {
                writer.writeCharacters(QString::fromUtf8(" "));
            }

            qreal lon = coordinates.longitude(GeoDataCoordinates::Degree);
            writer.writeCharacters(QString::number(lon, 'f', 10));
            writer.writeCharacters(QString::fromUtf8(","));
            qreal lat = coordinates.latitude(GeoDataCoordinates::Degree);
            writer.writeCharacters(QString::number(lat, 'f', 10));

            if (hasAltitude)
            {
                qreal alt = coordinates.altitude();
                writer.writeCharacters(QString::fromUtf8(","));
                writer.writeCharacters(QString::number(alt, 'f', 2));
            }
        }

        writer.writeEndElement();
        writer.writeEndElement();

        return true;
    }

    return false;
}

} // namespace Marble
