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

#include "KmlNetworkLinkControlTagWriter.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "GeoDataNetworkLinkControl.h"
#include "GeoDataTypes.h"
#include "GeoDataUpdate.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerNetworkLinkControl(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataNetworkLinkControlType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlNetworkLinkControlTagWriter);

bool KmlNetworkLinkControlTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataNetworkLinkControl* networkLinkControl = static_cast<const GeoDataNetworkLinkControl*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_NetworkLinkControl));
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_minRefreshPeriod), QString::number(networkLinkControl->minRefreshPeriod()), QString::fromUtf8("1"));
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_maxSessionLength), QString::number(networkLinkControl->maxSessionLength()), QString::fromUtf8("2"));
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_cookie), networkLinkControl->cookie());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_message), networkLinkControl->message());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_linkName), networkLinkControl->linkName());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_linkDescription), networkLinkControl->linkDescription());

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_linkSnippet));

    if (networkLinkControl->maxLines() > 0)
    {
        writer.writeAttribute(QString::fromUtf8("maxLines"), QString::number(networkLinkControl->maxLines()));
    }

    writer.writeCharacters(networkLinkControl->linkSnippet());
    writer.writeEndElement();

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_expires), networkLinkControl->expires().toString(Qt::ISODate));
    writeElement(&networkLinkControl->update(), writer);
    writer.writeEndElement();

    return true;
}

} // namespace Marble
