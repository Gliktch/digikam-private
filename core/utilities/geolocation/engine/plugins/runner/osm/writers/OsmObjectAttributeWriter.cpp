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

#include "OsmObjectAttributeWriter.h"

// Local includes

#include "OsmObjectManager.h"
#include "OsmPlacemarkData.h"
#include "GeoWriter.h"

namespace Marble
{

void OsmObjectAttributeWriter::writeAttributes(const OsmPlacemarkData& osmData, GeoWriter& writer)
{
    writer.writeAttribute(QLatin1String("id"), QString::number(osmData.id()));
    writer.writeOptionalAttribute(QLatin1String("action"), osmData.action());
    writer.writeOptionalAttribute(QLatin1String("changeset"), osmData.changeset());
    writer.writeOptionalAttribute(QLatin1String("timestamp"), osmData.timestamp());
    writer.writeOptionalAttribute(QLatin1String("uid"), osmData.uid());
    writer.writeOptionalAttribute(QLatin1String("user"), osmData.user());
    writer.writeOptionalAttribute(QLatin1String("version"), osmData.version());
    writer.writeOptionalAttribute(QLatin1String("visible"), osmData.isVisible());
}

} // namespace Marble
