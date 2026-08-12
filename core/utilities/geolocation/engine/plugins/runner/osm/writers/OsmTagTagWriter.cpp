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

#include "OsmTagTagWriter.h"

// Local includes

#include "GeoWriter.h"
#include "OsmElementDictionary.h"
#include "OsmPlacemarkData.h"

namespace Marble
{

QSet<QString> OsmTagTagWriter::m_blacklist;

void OsmTagTagWriter::writeTags(const OsmPlacemarkData& osmData, GeoWriter& writer)
{
    if (m_blacklist.isEmpty())
    {
        m_blacklist << QStringLiteral("mx:version");
        m_blacklist << QStringLiteral("mx:changeset");
        m_blacklist << QStringLiteral("mx:uid");
        m_blacklist << QStringLiteral("mx:visible");
        m_blacklist << QStringLiteral("mx:user");
        m_blacklist << QStringLiteral("mx:timestamp");
        m_blacklist << QStringLiteral("mx:action");
    }

    auto it = osmData.tagsBegin();
    auto end = osmData.tagsEnd();

    for (; it != end; ++it)
    {
        if (!m_blacklist.contains(it.key()))
        {
            writer.writeStartElement(QString::fromUtf8(osm::osmTag_tag));
            writer.writeAttribute(QLatin1String("k"), it.key());
            writer.writeAttribute(QLatin1String("v"), it.value());
            writer.writeEndElement();
        }
    }
}

} // namespace Marble
