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

#include "KmlLinearRingTagWriter.h"

// Local includes

#include "GeoDataLinearRing.h"
#include "GeoDataTypes.h"
#include "GeoDataCoordinates.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLookAt(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataLinearRingType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlLinearRingTagWriter);

bool KmlLinearRingTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataLinearRing* ring = static_cast<const GeoDataLinearRing*>(node);

    if (ring->size() > 1)
    {
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_LinearRing));
        KmlObjectTagWriter::writeIdentifiers(writer, ring);
        writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_extrude), QString::number(ring->extrude()), QString::fromUtf8("0"));
        writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_tessellate), QString::number(ring->tessellate()), QString::fromUtf8("0"));
        writer.writeStartElement(QString::fromUtf8("coordinates"));

        int size = ring->size() >= 3 && ring->first() != ring->last() ? ring->size() + 1 : ring->size();

        for (int i = 0; i < size; ++i)
        {
            GeoDataCoordinates coordinates = ring->at(i % ring->size());

            if (i > 0)
            {
                writer.writeCharacters(QString::fromUtf8(" "));
            }

            qreal lon = coordinates.longitude(GeoDataCoordinates::Degree);
            writer.writeCharacters(QString::number(lon, 'f', 10));
            writer.writeCharacters(QString::fromUtf8(","));
            qreal lat = coordinates.latitude(GeoDataCoordinates::Degree);
            writer.writeCharacters(QString::number(lat, 'f', 10));
        }

        writer.writeEndElement();
        writer.writeEndElement();

        return true;
    }

    return false;
}

} // namespace Marble
