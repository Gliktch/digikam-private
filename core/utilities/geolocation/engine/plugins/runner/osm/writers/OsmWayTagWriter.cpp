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

#include "OsmWayTagWriter.h"

// Local includes

#include "OsmElementDictionary.h"
#include "OsmObjectAttributeWriter.h"
#include "OsmTagTagWriter.h"
#include "GeoDataLineString.h"
#include "GeoDataCoordinates.h"
#include "OsmPlacemarkData.h"
#include "OsmObjectManager.h"
#include "GeoWriter.h"

namespace Marble
{

void OsmWayTagWriter::writeWay(const GeoDataLineString& lineString,
                               const OsmPlacemarkData& osmData, GeoWriter& writer)
{

    writer.writeStartElement(QString::fromUtf8(osm::osmTag_way));

    OsmObjectAttributeWriter::writeAttributes(osmData, writer);
    OsmTagTagWriter::writeTags(osmData, writer);

    // Writing all the component nodes ( Nd tags )
    QVector<GeoDataCoordinates>::const_iterator it =  lineString.constBegin();
    QVector<GeoDataCoordinates>::ConstIterator const end = lineString.constEnd();

    for (; it != end; ++it)
    {
        QString ndId = QString::number(osmData.nodeReference(*it).id());
        writer.writeStartElement(QString::fromUtf8(osm::osmTag_nd));
        writer.writeAttribute(QLatin1String("ref"), ndId);
        writer.writeEndElement();
    }

    if (!lineString.isEmpty() && lineString.isClosed())
    {
        auto const startId = osmData.nodeReference(lineString.first()).id();
        auto const endId = osmData.nodeReference(lineString.last()).id();

        if (startId != endId)
        {
            writer.writeStartElement(QString::fromUtf8(osm::osmTag_nd));
            writer.writeAttribute(QLatin1String("ref"), QString::number(startId));
            writer.writeEndElement();
        }
    }

    writer.writeEndElement();
}

} // namespace Marble
