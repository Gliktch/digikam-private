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

#include "KmlStyleUrlTagHandler.h"

// Qt includes

#include <QString>

// Local includes

#include "KmlElementDictionary.h"
#include "GeoDataFeature.h"
#include "GeoDataStyleMap.h"
#include "GeoParser.h"
#include "digikam_debug.h"

namespace Marble
{

namespace kml
{

KML_DEFINE_TAG_HANDLER(styleUrl)

GeoNode* KmlstyleUrlTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_styleUrl)));

    GeoStackItem parentItem = parser.parentElement();
    GeoStackItem grandParentItem = parser.parentElement();

    if (parentItem.represents(kmlTag_Pair))
    {
        QString content = parser.readElementText().trimmed();

        QString key = parentItem.nodeAs<GeoDataStyleMap>()->lastKey();
        (*parentItem.nodeAs<GeoDataStyleMap>())[ key ] = content;
        parentItem.nodeAs<GeoDataStyleMap>()->setLastKey(QString());

    }

    else if (parentItem.is<GeoDataFeature>())
    {
        QString content = parser.readElementText().trimmed();

        parentItem.nodeAs<GeoDataFeature>()->setStyleUrl(content);
    }

    return nullptr;
}

} // namespace kml

} // namespace Marble
