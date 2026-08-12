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

#pragma once

// Local includes

#include "GeoDataCoordinates.h"

namespace Marble
{

class GeoDataLineString;
class GeoDataDocument;
class GeoDataLinearRing;
class GeoDataPolygon;
class GeoDataPlacemark;
class GeoDataFeature;
class OsmPlacemarkData;

class OsmConverter
{
public:

    typedef QPair<QString, QString> Tag;
    typedef QPair<GeoDataCoordinates, OsmPlacemarkData > Node;
    typedef QPair<const GeoDataLineString*, OsmPlacemarkData > Way;
    typedef QPair<const GeoDataFeature*, OsmPlacemarkData > Relation;

    using Nodes     = QVector<Node>;
    using Tags      = QVector<Tag>;
    using Ways      = QVector<Way>;
    using Relations = QVector<Relation>;

public:

    void read(const GeoDataDocument* document);

    const Nodes& nodes() const;
    const Ways& ways() const;
    const Relations& relations() const;

private:

    void processLinearRing(GeoDataLinearRing* linearRing,
                           const OsmPlacemarkData& osmData);
    void processPolygon(GeoDataPolygon* polygon,
                        const OsmPlacemarkData& osmData,
                        GeoDataPlacemark* placemark);

private:

    Nodes       m_nodes;
    Ways        m_ways;
    Relations   m_relations;
};

} // namespace Marble
