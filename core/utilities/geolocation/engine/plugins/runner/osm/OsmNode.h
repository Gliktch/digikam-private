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

#include <QString>

// Local includes

#include "OsmPlacemarkData.h"
#include "GeoDataPlacemark.h"

class QXmlStreamAttributes;

namespace Marble
{

class GeoDataDocument;

class OsmNode
{

public:

    OsmPlacemarkData& osmData();
    void parseCoordinates(const QXmlStreamAttributes& attributes);
    void setCoordinates(const GeoDataCoordinates& coordinates);

    const GeoDataCoordinates& coordinates() const;
    const OsmPlacemarkData& osmData() const;

    GeoDataPlacemark* create() const;

private:

    int populationIndex(qint64 population) const;

private:

    OsmPlacemarkData   m_osmData;
    GeoDataCoordinates m_coordinates;
};

typedef QHash<qint64, OsmNode> OsmNodes;

} // namespace Marble
