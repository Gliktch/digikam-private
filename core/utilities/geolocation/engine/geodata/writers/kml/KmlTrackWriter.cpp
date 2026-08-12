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

#include "KmlTrackWriter.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "GeoDataCoordinates.h"
#include "GeoDataTrack.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerPoint(GeoTagWriter::QualifiedName(
                                               QString::fromUtf8(GeoDataTypes::GeoDataTrackType),
                                               QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                           new KmlTrackWriter());

bool KmlTrackWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataTrack* track = static_cast<const GeoDataTrack*>(node);

    writer.writeStartElement(QString::fromUtf8("gx:Track"));
    KmlObjectTagWriter::writeIdentifiers(writer, track);

    int points = track->size();

    for (int i = 0; i < points; i++)
    {
        writer.writeElement(QString::fromUtf8("when"), track->whenList().at(i).toString(Qt::ISODate));

        qreal lon, lat, alt;
        track->coordinatesList().at(i).geoCoordinates(lon, lat, alt, GeoDataCoordinates::Degree);
        const QString coord = QString::number(lon, 'f', 10) + QLatin1Char(' ') +
                              QString::number(lat, 'f', 10) + QLatin1Char(' ') +
                              QString::number(alt, 'f', 10);

        writer.writeElement(QString::fromUtf8("gx:coord"), coord);
    }

    writer.writeEndElement();

    return true;
}

} // namespace Marble
