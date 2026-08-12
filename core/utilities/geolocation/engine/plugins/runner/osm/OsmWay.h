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

// Qt includes

#include <QSet>
#include <QString>

// Local includes

#include "OsmNode.h"
#include "GeoDataBuilding.h"
#include "OsmPlacemarkData.h"
#include "StyleBuilder.h"

namespace Marble
{

class GeoDataDocument;

class OsmWay
{
public:

    OsmPlacemarkData& osmData();
    void addReference(qint64 id);

    const OsmPlacemarkData& osmData() const;
    const QVector<qint64>& references() const;

    GeoDataPlacemark* create(const OsmNodes& nodes, QSet<qint64>& usedNodes) const;

private:

    bool isArea() const;

    static bool isAreaTag(const StyleBuilder::OsmTag& keyValue);

    bool isBuilding() const;

    static bool isBuildingTag(const StyleBuilder::OsmTag& keyValue);


    QString extractBuildingName() const;
    double extractBuildingHeight() const;
    QVector<GeoDataBuilding::NamedEntry> extractNamedEntries() const;

private:

    OsmPlacemarkData                  m_osmData;
    QVector<qint64>                   m_references;

    static QSet<StyleBuilder::OsmTag> s_areaTags;
    static QSet<StyleBuilder::OsmTag> s_buildingTags;
};

typedef QHash<qint64, OsmWay> OsmWays;

} // namespace Marble
