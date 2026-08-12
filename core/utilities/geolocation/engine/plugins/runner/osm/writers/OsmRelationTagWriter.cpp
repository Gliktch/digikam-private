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

#include "OsmRelationTagWriter.h"

// Local includes

#include "OsmElementDictionary.h"
#include "OsmObjectAttributeWriter.h"
#include "OsmTagTagWriter.h"
#include "GeoDataLineString.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPolygon.h"
#include "GeoDataCoordinates.h"
#include "GeoDataPlacemark.h"
#include "GeoWriter.h"
#include "OsmPlacemarkData.h"
#include "OsmObjectManager.h"

namespace Marble
{

void OsmRelationTagWriter::writeMultipolygon(const GeoDataPolygon& polygon,
                                             const OsmPlacemarkData& osmData, GeoWriter& writer)
{
    writer.writeStartElement(QString::fromUtf8(osm::osmTag_relation));

    OsmObjectAttributeWriter::writeAttributes(osmData, writer);
    OsmTagTagWriter::writeTags(osmData, writer);

    writer.writeStartElement(QString::fromUtf8(osm::osmTag_member));
    QString memberId = QString::number(osmData.memberReference(-1).id());
    writer.writeAttribute(QLatin1String("type"), QLatin1String("way"));
    writer.writeAttribute(QLatin1String("ref"),  memberId);
    writer.writeAttribute(QLatin1String("role"), QLatin1String("outer"));
    writer.writeEndElement();

    qsizetype size = polygon.innerBoundaries().size();

    for (int index = 0 ; index < size ; ++index)
    {
        writer.writeStartElement(QString::fromUtf8(osm::osmTag_member));
        QString memberId2 = QString::number(osmData.memberReference(index).id());
        writer.writeAttribute(QLatin1String("type"), QLatin1String("way"));
        writer.writeAttribute(QLatin1String("ref"),  memberId2);
        writer.writeAttribute(QLatin1String("role"), QLatin1String("inner"));
        writer.writeEndElement();
    }

    writer.writeEndElement();
}

void OsmRelationTagWriter::writeRelation(const QVector<GeoDataPlacemark*>& members, const QString& id,
                                         const QMap<QString, QString>& tags, GeoWriter& writer)
{
    Q_UNUSED(members);
    Q_UNUSED(id);
    Q_UNUSED(tags);
    Q_UNUSED(writer);

    // Not implemented yet ( need to implement the custom relations first )
}

} // namespace Marble
