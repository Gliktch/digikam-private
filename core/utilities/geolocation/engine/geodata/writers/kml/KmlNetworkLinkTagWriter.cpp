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

#include "KmlNetworkLinkTagWriter.h"

// Local includes

#include "GeoDataNetworkLink.h"
#include "GeoDataTypes.h"
#include "GeoDataLink.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerNetworkLink(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataNetworkLinkType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlNetworkLinkTagWriter);

bool KmlNetworkLinkTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{

    const GeoDataNetworkLink* networkLink = static_cast<const GeoDataNetworkLink*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_NetworkLink));

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_name), networkLink->name());

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_visibility), QString::number(networkLink->isVisible()), QString::fromUtf8("1"));

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_refreshVisibility), QString::number(networkLink->refreshVisibility()), QString::fromUtf8("0"));

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_flyToView), QString::number(networkLink->flyToView()), QString::fromUtf8("0"));

    writeElement(&networkLink->link(), writer);

    writer.writeEndElement();

    return true;
}

} // namespace Marble
