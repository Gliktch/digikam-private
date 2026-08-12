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

#include "OsmDocumentTagTranslator.h"

// Qt includes

#include <QDebug>

// Local includes

#include "OsmNodeTagWriter.h"
#include "OsmWayTagWriter.h"
#include "OsmElementDictionary.h"
#include "GeoDataDocument.h"
#include "GeoWriter.h"
#include "GeoDataGeometry.h"
#include "GeoDataPoint.h"
#include "GeoDataPolygon.h"
#include "GeoDataBuilding.h"
#include "GeoDataMultiGeometry.h"
#include "GeoDataPlacemark.h"
#include "GeoDataLinearRing.h"
#include "GeoDataTypes.h"
#include "OsmPlacemarkData.h"
#include "OsmObjectManager.h"
#include "OsmRelationTagWriter.h"
#include "OsmConverter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerDocument(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataDocumentType),
                                                                          QString::fromUtf8(osm::osmTag_version06)),
                                              new OsmDocumentTagTranslator());


bool OsmDocumentTagTranslator::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataDocument* document = static_cast<const GeoDataDocument*>(node);

    OsmConverter converter;
    converter.read(document);
    OsmNodeTagWriter::writeAllNodes(converter.nodes(), writer);

    qint64 lastId                   = 0;

    for (auto const& way : converter.ways())
    {
        if (way.second.id() != lastId)
        {
            OsmWayTagWriter::writeWay(*way.first, way.second, writer);
            lastId = way.second.id();
        }
    }

    for (auto const& relation : converter.relations())
    {
        if (auto placemark = geodata_cast<GeoDataPlacemark>(relation.first))
        {
            // cppcheck-suppress constVariablePointer
            if (const auto building = geodata_cast<GeoDataBuilding>(placemark->geometry()))
            {
                auto polygon = geodata_cast<GeoDataPolygon>(&building->multiGeometry()->at(0));

                Q_ASSERT(polygon);

                OsmRelationTagWriter::writeMultipolygon(*polygon, relation.second, writer);
            }
            else
            {
                auto polygon = geodata_cast<GeoDataPolygon>(placemark->geometry());

                Q_ASSERT(polygon);

                OsmRelationTagWriter::writeMultipolygon(*polygon, relation.second, writer);
            }
        }
    }

    return true;
}

} // namespace Marble
