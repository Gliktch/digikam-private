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

#include "GeoTagWriter.h"

namespace Marble
{

/**
     This helper class organizes a GeoDataDocument to
     meet the following OSM standard structure:
@code
     <xml>
     <bounds>

     block of nodes: ( <node>...) 1st
     block of ways:  ( <way> ...) 2nd
     block of relations: ( <relation> ...) 3rd
@endcode

     See https://wiki.openstreetmap.org/wiki/OSM_XML#Contents

*/
class OsmDocumentTagTranslator : public GeoTagWriter
{
public:

    bool write(const GeoNode* node, GeoWriter& writer) const override;
};

} // namespace Marble
