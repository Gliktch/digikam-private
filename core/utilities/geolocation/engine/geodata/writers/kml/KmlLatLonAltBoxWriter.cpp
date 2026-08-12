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

#include "KmlLatLonAltBoxWriter.h"

// Local includes

#include "GeoDataLatLonAltBox.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlGroundOverlayWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLatLonAltBox(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataLatLonAltBoxType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlLatLonAltBoxWriter);

bool KmlLatLonAltBoxWriter::write(const GeoNode* node,
                                  GeoWriter& writer) const
{
    const GeoDataLatLonAltBox* latLonAltBox = static_cast<const GeoDataLatLonAltBox*>(node);
    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_LatLonAltBox));
    KmlObjectTagWriter::writeIdentifiers(writer, latLonAltBox);
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_north), QString::number(latLonAltBox->north(GeoDataCoordinates::Degree)));
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_south), QString::number(latLonAltBox->south(GeoDataCoordinates::Degree)));
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_east),  QString::number(latLonAltBox->east(GeoDataCoordinates::Degree)));
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_west),  QString::number(latLonAltBox->west(GeoDataCoordinates::Degree)));
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_minAltitude), QString::number(latLonAltBox->minAltitude()));
    writer.writeTextElement(QString::fromUtf8(kml::kmlTag_maxAltitude), QString::number(latLonAltBox->maxAltitude()));
    KmlGroundOverlayWriter::writeAltitudeMode(writer, latLonAltBox->altitudeMode());
    writer.writeEndElement();
    return true;
}

} // namespace Marble
