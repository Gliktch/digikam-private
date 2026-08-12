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

#include "KmlPointTagWriter.h"

// Local includes

#include "GeoDataPoint.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlGroundOverlayWriter.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerPoint(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataPointType),
                                                                       QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                           new KmlPointTagWriter());


bool KmlPointTagWriter::write(const GeoNode* node,
                              GeoWriter& writer) const
{
    const GeoDataPoint* point = static_cast<const GeoDataPoint*>(node);

    if (!point->coordinates().isValid())
    {
        return true;
    }

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Point));
    KmlObjectTagWriter::writeIdentifiers(writer, point);
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_extrude), QString::number(point->extrude()), QString::fromUtf8("0"));
    writer.writeStartElement(QString::fromUtf8("coordinates"));

    //FIXME: this should be using the GeoDataCoordinates::toString but currently
    // it is not including the altitude and is adding an extra space after commas

    QString coordinateString =
        QString::number(point->coordinates().longitude(GeoDataCoordinates::Degree), 'f', 10) +
        QLatin1Char(',') +
        QString::number(point->coordinates().latitude(GeoDataCoordinates::Degree), 'f', 10);

    if (point->coordinates().altitude())
    {
        coordinateString += QLatin1Char(',') + QString::number(point->coordinates().altitude(), 'f', 10);
    }

    writer.writeCharacters(coordinateString);
    writer.writeEndElement();

    KmlGroundOverlayWriter::writeAltitudeMode(writer, point->altitudeMode());

    writer.writeEndElement();

    return true;
}

} // namespace Marble
