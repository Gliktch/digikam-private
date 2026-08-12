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

#include "OsmNode.h"
#include "OsmWay.h"
#include "OsmRelation.h"

namespace OSMPBF
{
class PrimitiveBlock;
class PrimitiveGroup;
}

namespace Marble
{

class OsmPbfParser
{
public:

    void parse(const uint8_t* data, std::size_t len);

public:

    OsmNodes     m_nodes;
    OsmWays      m_ways;
    OsmRelations m_relations;

private:

    bool parseBlob(const uint8_t*& it, const uint8_t* end);
    void parsePrimitiveBlock(const uint8_t* data, std::size_t len);
    void parseDenseNodes(const OSMPBF::PrimitiveBlock& block,  const OSMPBF::PrimitiveGroup& group);
    void parseWays(const OSMPBF::PrimitiveBlock& block, const OSMPBF::PrimitiveGroup& group);
    void parseRelations(const OSMPBF::PrimitiveBlock& block, const OSMPBF::PrimitiveGroup& group);

private:

    QByteArray    m_zlibBuffer;
    QSet<QString> m_stringPool;
};

} // namespace Marble
