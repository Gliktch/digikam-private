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

#include "KmlListStyleTagWriter.h"

// Local includes

#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlColorStyleTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerListStyle(GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataListStyleType),
                                                                           QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
                                               new KmlListStyleTagWriter());

bool KmlListStyleTagWriter::write(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataListStyle* listStyle = static_cast<const GeoDataListStyle*>(node);

    bool const isEmpty = (
                          (listStyle->listItemType()    == GeoDataListStyle::Check) &&
                          (listStyle->backgroundColor() == QColor(Qt::white))       &&
                          listStyle->itemIconList().isEmpty()
                         );

    if (isEmpty)
    {
        return true;
    }

    writer.writeStartElement(QString::fromUtf8(kml::kmlTag_ListStyle));

    QString const itemType  = itemTypeToString(listStyle->listItemType());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_listItemType), itemType, QString::fromUtf8("check"));
    QString const color     = KmlColorStyleTagWriter::formatColor(listStyle->backgroundColor());
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_bgColor), color, QString::fromUtf8("ffffffff"));

    for (const GeoDataItemIcon* icon : listStyle->itemIconList())
    {
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_ItemIcon));
        QString const state = iconStateToString(icon->state());
        writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_state), state, QString::fromUtf8("open"));
        writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_href), icon->iconPath());
        writer.writeEndElement();
    }

    writer.writeEndElement();

    return true;
}

QString KmlListStyleTagWriter::itemTypeToString(GeoDataListStyle::ListItemType itemType)
{
    switch (itemType)
    {
        case GeoDataListStyle::CheckOffOnly:
        {
            return QString::fromUtf8("checkOffOnly");
        }

        case GeoDataListStyle::CheckHideChildren:
        {
            return QString::fromUtf8("checkHideChildren");
        }

        case GeoDataListStyle::RadioFolder:
        {
            return QString::fromUtf8("radioFolder");
        }

        default:
        {
            return QString::fromUtf8("check");
        }
    }
}

QString KmlListStyleTagWriter::iconStateToString(GeoDataItemIcon::ItemIconStates state)
{
    QStringList stateList;

    if (state & GeoDataItemIcon::Open)
    {
        stateList << QString::fromUtf8("open");
    }

    if (state & GeoDataItemIcon::Closed)
    {
        stateList << QString::fromUtf8("closed");
    }

    if (state & GeoDataItemIcon::Error)
    {
        stateList << QString::fromUtf8("error");
    }

    if (state & GeoDataItemIcon::Fetching0)
    {
        stateList << QString::fromUtf8("fetching0");
    }

    if (state & GeoDataItemIcon::Fetching1)
    {
        stateList << QString::fromUtf8("fetching1");
    }

    if (state & GeoDataItemIcon::Fetching2)
    {
        stateList << QString::fromUtf8("fetching2");
    }

    return stateList.join(QLatin1Char(' '));
}

} // namespace Marble
