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
#include <QXmlStreamAttributes>
#include <QSet>

// Local includes

#include "OsmNode.h"
#include "OsmWay.h"
#include "OsmPlacemarkData.h"
#include "GeoDataLinearRing.h"

namespace Marble
{

class GeoDataDocument;

class OsmRelation
{
public:

    OsmPlacemarkData& osmData();
    void parseMember(const QXmlStreamAttributes& attributes);
    void addMember(qint64 reference, const QString& role, const QString& type);
    void createMultipolygon(GeoDataDocument* document, OsmWays& ways, const OsmNodes& nodes, QSet<qint64>& usedNodes, QSet<qint64>& usedWays) const;
    void createRelation(GeoDataDocument* document, const QHash<qint64, GeoDataPlacemark*>& wayPlacemarks) const;

    const OsmPlacemarkData& osmData() const;

private:

    typedef QPair<GeoDataLinearRing, OsmPlacemarkData> OsmRing;
    using OsmRings = QVector<OsmRing>;

    struct OsmMember
    {
        QString type;
        QString role;
        qint64 reference;

        OsmMember();
    };

private:

    OsmRings rings(const QStringList& roles, const OsmWays& ways, const OsmNodes& nodes, QSet<qint64>& usedNodes, QSet<qint64>& usedWays) const;

private:

    OsmPlacemarkData   m_osmData;
    QVector<OsmMember> m_members;
};

typedef QHash<qint64, OsmRelation> OsmRelations;

} // namespace Marble
