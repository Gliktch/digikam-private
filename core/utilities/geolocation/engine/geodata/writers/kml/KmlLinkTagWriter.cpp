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

#include "KmlLinkTagWriter.h"

// Local includes

#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLink(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataLinkType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlLinkTagWriter);

bool KmlLinkTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataLink* link = static_cast<const GeoDataLink*>(node);

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Link));
    KmlObjectTagWriter::writeIdentifiers(writer, link);

    writer.writeElement(QString::fromUtf8(kml::kmlTag_href), link->href());

    QString const refreshMode = refreshModeToString(link->refreshMode());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_refreshMode), refreshMode, QString::fromUtf8("onChange"));

    writer.writeElement(QString::fromUtf8(kml::kmlTag_refreshInterval), QString::number(link->refreshInterval()));

    QString const viewRefreshMode = viewRefreshModeToString(link->viewRefreshMode());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_viewRefreshMode), viewRefreshMode, QString::fromUtf8("never"));

    writer.writeElement(QString::fromUtf8(kml::kmlTag_viewRefreshTime), QString::number(link->viewRefreshTime()));

    writer.writeElement(QString::fromUtf8(kml::kmlTag_viewBoundScale), QString::number(link->viewBoundScale()));

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_viewFormat), link->viewFormat());

    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_httpQuery), link->httpQuery());

    writer.writeEndElement();

    return true;
}

QString KmlLinkTagWriter::refreshModeToString(GeoDataLink::RefreshMode refreshMode)
{
    switch (refreshMode)
    {
        case GeoDataLink::OnInterval:
            return QString::fromUtf8("onInterval");

        case GeoDataLink::OnExpire:
            return QString::fromUtf8("onExpire");

        default:
            return QString::fromUtf8("onChange");
    }
}

QString KmlLinkTagWriter::viewRefreshModeToString(GeoDataLink::ViewRefreshMode viewRefreshMode)
{
    switch (viewRefreshMode)
    {
        case GeoDataLink::OnStop:
            return QString::fromUtf8("onStop");

        case GeoDataLink::OnRequest:
            return QString::fromUtf8("onRequest");

        case GeoDataLink::OnRegion:
            return QString::fromUtf8("onRegion");

        default:
            return QString::fromUtf8("never");
    }
}

} // namespace Marble
