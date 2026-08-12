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

#include "OsmNodeTagWriter.h"

// Qt includes

#include <QHash>

// Local includes

#include "OsmElementDictionary.h"
#include "OsmObjectAttributeWriter.h"
#include "OsmTagTagWriter.h"
#include "GeoDataPoint.h"
#include "GeoDataLineString.h"
#include "GeoWriter.h"
#include "OsmPlacemarkData.h"
#include "OsmObjectManager.h"

namespace Marble
{

void OsmNodeTagWriter::writeNode(const OsmConverter::Node& node, GeoWriter& writer)
{
    QString lat = QString::number(node.first.latitude(GeoDataCoordinates::Degree), 'f', 7);
    QString lon = QString::number(node.first.longitude(GeoDataCoordinates::Degree), 'f', 7);

    writer.writeStartElement(QLatin1String(osm::osmTag_node));

    writer.writeAttribute(QLatin1String("lat"), lat);
    writer.writeAttribute(QLatin1String("lon"), lon);
    OsmObjectAttributeWriter::writeAttributes(node.second, writer);
    OsmTagTagWriter::writeTags(node.second, writer);

    writer.writeEndElement();
}

void OsmNodeTagWriter::writeAllNodes(const OsmConverter::Nodes& nodes, GeoWriter& writer)
{
    // Writing all the component nodes
    qint64 lastId = 0;

    for (auto const& node : nodes)
    {
        if (node.second.id() != lastId)
        {
            writeNode(node, writer);
            lastId = node.second.id();
        } // else duplicate/shared node
    }
}

} // namespace Marble
