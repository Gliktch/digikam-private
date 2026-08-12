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

#include "DgmlItemTagHandler.h"

// Local includes

#include "DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "DgmlAuxillaryDictionary.h"
#include "GeoParser.h"
#include "GeoSceneSection.h"
#include "GeoSceneItem.h"
#include "digikam_debug.h"

namespace Marble
{

namespace dgml
{

DGML_DEFINE_TAG_HANDLER(Item)

GeoNode* DgmlItemTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Item)));

    QString name      = parser.attribute(dgmlAttr_name).trimmed();
    QString checkable = parser.attribute(dgmlAttr_checkable).toLower().trimmed();
    QString connectTo = parser.attribute(dgmlAttr_connect).trimmed();
    int     spacing   = parser.attribute(dgmlAttr_spacing).toInt();

    GeoSceneItem* item = nullptr;

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(dgmlTag_Section))
    {
        item = new GeoSceneItem(name);
        item->setCheckable(checkable == QString::fromUtf8(dgmlValue_true) || checkable == QString::fromUtf8(dgmlValue_on));
        item->setConnectTo(connectTo);
        item->setSpacing(spacing);
        parentItem.nodeAs<GeoSceneSection>()->addItem(item);
    }

    return item;
}

} // namespace dgml

} // namespace Marble
